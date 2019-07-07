/***********************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2009, Percona Inc.

Portions of this file contain modifications contributed and copyrighted
by Percona Inc.. Those modifications are
gratefully acknowledged and are described briefly in the InnoDB
documentation. The contributions by Percona Inc. are incorporated with
their permission, and subject to the conditions contained in the file
COPYING.Percona.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

***********************************************************************/

/** @file os/os0file.cc
 The interface to the operating system file i/o primitives

 Created 10/21/1995 Heikki Tuuri
 *******************************************************/

#include <innodb/time/ut_time.h>
#include <innodb/logger/info.h>
#include <innodb/sync_event/os_event_wait.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_is_set.h>
#include <innodb/ioasync/Slot.h>
#include <innodb/ioasync/AIO.h>
#include <innodb/ioasync/os_aio_n_segments.h>

#include "os0file.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "sql_const.h"
#include "srv0start.h"


#ifdef UNIV_HOTBACKUP
#include "fil0fil.h"
#include "fsp0types.h"

/** MEB routine to get the master key. MEB will extract
the key from the keyring encrypted file stored in backup.
@param[in]	key_id		the id of the master key
@param[in]	key_type	master key type
@param[out]	key		the master key being returned
@param[out]	key_length	the length of the returned key
@retval	0 if the key is being returned, 1 otherwise. */
extern int meb_key_fetch(const char *key_id, char **key_type,
                         const char *user_id, void **key, size_t *key_length);
#endif /* UNIV_HOTBACKUP */


#ifdef _WIN32
#include <errno.h>
#include <mbstring.h>
#include <sys/stat.h>
#include <tchar.h>
#include <codecvt>
#endif /* _WIN32 */

#ifdef __linux__
#include <sys/sendfile.h>
#endif /* __linux__ */

#ifdef LINUX_NATIVE_AIO
#ifndef UNIV_HOTBACKUP
#include <libaio.h>
#else /* !UNIV_HOTBACKUP */
#undef LINUX_NATIVE_AIO
#endif /* !UNIV_HOTBACKUP */
#endif /* LINUX_NATIVE_AIO */

#ifdef HAVE_FALLOC_PUNCH_HOLE_AND_KEEP_SIZE
#include <fcntl.h>
#include <linux/falloc.h>
#endif /* HAVE_FALLOC_PUNCH_HOLE_AND_KEEP_SIZE */

#include <errno.h>
#include <lz4.h>
#include "my_aes.h"
#include "my_rnd.h"
#include "mysql/service_mysql_keyring.h"
#include "mysqld.h"

#include <sys/types.h>
#include <zlib.h>
#include <ctime>
#include <functional>
#include <new>
#include <vector>

#include <innodb/io/Block.h>
#include <innodb/io/Blocks.h>
#include <innodb/io/block_cache.h>


#ifdef UNIV_HOTBACKUP
#include <data0type.h>
#endif /* UNIV_HOTBACKUP */

/* Flush after each os_fsync_threshold bytes */
unsigned long long os_fsync_threshold = 0;









/** Disk sector size of aligning write buffer for DIRECT_IO */
static ulint os_io_ptr_align = UNIV_SECTOR_SIZE;

#ifdef _WIN32

/* On Windows when using native AIO the number of AIO requests
that a thread can handle at a given time is limited to 32
i.e.: SRV_N_PENDING_IOS_PER_THREAD */
#define SRV_N_PENDING_IOS_PER_THREAD OS_AIO_N_PENDING_IOS_PER_THREAD

#endif /* _WIN32 */

/** In simulated aio, merge at most this many consecutive i/os */
static const ulint OS_AIO_MERGE_N_CONSECUTIVE = 64;

/** Flag indicating if the page_cleaner is in active state. */
extern bool buf_page_cleaner_is_active;

#ifndef UNIV_HOTBACKUP
/**********************************************************************

InnoDB AIO Implementation:
=========================

We support native AIO for Windows and Linux. For rest of the platforms
we simulate AIO by special IO-threads servicing the IO-requests.

Simulated AIO:
==============

On platforms where we 'simulate' AIO, the following is a rough explanation
of the high level design.
There are four io-threads (for ibuf, log, read, write).
All synchronous IO requests are serviced by the calling thread using
os_file_write/os_file_read. The Asynchronous requests are queued up
in an array (there are four such arrays) by the calling thread.
Later these requests are picked up by the IO-thread and are serviced
synchronously.

Windows native AIO:
==================

If srv_use_native_aio is not set then Windows follow the same
code as simulated AIO. If the flag is set then native AIO interface
is used. On windows, one of the limitation is that if a file is opened
for AIO no synchronous IO can be done on it. Therefore we have an
extra fifth array to queue up synchronous IO requests.
There are innodb_file_io_threads helper threads. These threads work
on the four arrays mentioned above in Simulated AIO. No thread is
required for the sync array.
If a synchronous IO request is made, it is first queued in the sync
array. Then the calling thread itself waits on the request, thus
making the call synchronous.
If an AIO request is made the calling thread not only queues it in the
array but also submits the requests. The helper thread then collects
the completed IO request and calls completion routine on it.

Linux native AIO:
=================

If we have libaio installed on the system and innodb_use_native_aio
is set to true we follow the code path of native AIO, otherwise we
do simulated AIO.
There are innodb_file_io_threads helper threads. These threads work
on the four arrays mentioned above in Simulated AIO.
If a synchronous IO request is made, it is handled by calling
os_file_write/os_file_read.
If an AIO request is made the calling thread not only queues it in the
array but also submits the requests. The helper thread then collects
the completed IO request and calls completion routine on it.

**********************************************************************/

#ifdef UNIV_PFS_IO
/* Keys to register InnoDB I/O with performance schema */
mysql_pfs_key_t innodb_log_file_key;
mysql_pfs_key_t innodb_data_file_key;
mysql_pfs_key_t innodb_temp_file_key;
mysql_pfs_key_t innodb_arch_file_key;
mysql_pfs_key_t innodb_clone_file_key;
#endif /* UNIV_PFS_IO */

#endif /* !UNIV_HOTBACKUP */




#if defined(LINUX_NATIVE_AIO)
/** timeout for each io_getevents() call = 500ms. */
static const ulint OS_AIO_REAP_TIMEOUT = 500000000UL;




#endif /* LINUX_NATIVE_AIO */

/** Array of events used in simulated AIO */
static os_event_t *os_aio_segment_wait_events = NULL;


/** If the following is true, read i/o handler threads try to
wait until a batch of new read requests have been posted */
static bool os_aio_recommend_sleep_for_read_threads = false;

ulint os_n_file_reads = 0;
static ulint os_bytes_read_since_printout = 0;
ulint os_n_file_writes = 0;
ulint os_n_fsyncs = 0;
static ulint os_n_file_reads_old = 0;
static ulint os_n_file_writes_old = 0;
static ulint os_n_fsyncs_old = 0;
/** Number of pending write operations */
ulint os_n_pending_writes = 0;
/** Number of pending read operations */
ulint os_n_pending_reads = 0;

static time_t os_last_printout;
bool os_has_said_disk_full = false;

/** Default Zip compression level */
extern uint page_zip_level;

static_assert(DATA_TRX_ID_LEN <= 6, "COMPRESSION_ALGORITHM will not fit!");

/** Validates the consistency of the aio system.
@return true if ok */
static bool os_aio_validate();


/** Does simulated AIO. This function should be called by an i/o-handler
thread.

@param[in]	global_segment	The number of the segment in the aio arrays to
                                await for; segment 0 is the ibuf i/o thread,
                                segment 1 the log i/o thread, then follow the
                                non-ibuf read threads, and as the last are the
                                non-ibuf write threads
@param[out]	m1		the messages passed with the AIO request; note
                                that also in the case where the AIO operation
                                failed, these output parameters are valid and
                                can be used to restart the operation, for
                                example
@param[out]	m2		Callback argument
@param[in]	type		IO context
@return DB_SUCCESS or error code */
static dberr_t os_aio_simulated_handler(ulint global_segment, fil_node_t **m1,
                                        void **m2, IORequest *type);

#ifdef WIN_ASYNC_IO
/** This function is only used in Windows asynchronous i/o.
Waits for an aio operation to complete. This function is used to wait the
for completed requests. The aio array of pending requests is divided
into segments. The thread specifies which segment or slot it wants to wait
for. NOTE: this function will also take care of freeing the aio slot,
therefore no other thread is allowed to do the freeing!
@param[in]	segment		The number of the segment in the aio arrays to
wait for; segment 0 is the ibuf I/O thread,
segment 1 the log I/O thread, then follow the
non-ibuf read threads, and as the last are the
non-ibuf write threads; if this is
ULINT_UNDEFINED, then it means that sync AIO
is used, and this parameter is ignored
@param[in]	pos		this parameter is used only in sync AIO:
wait for the aio slot at this position
@param[out]	m1		the messages passed with the AIO request; note
that also in the case where the AIO operation
failed, these output parameters are valid and
can be used to restart the operation,
for example
@param[out]	m2		callback message
@param[out]	type		OS_FILE_WRITE or ..._READ
@return DB_SUCCESS or error code */
static dberr_t os_aio_windows_handler(ulint segment, ulint pos, fil_node_t **m1,
                                      void **m2, IORequest *type);
#endif /* WIN_ASYNC_IO */


/** Initializes the asynchronous io system. Creates one array each for ibuf
and log i/o. Also creates one array each for read and write where each
array is divided logically into n_readers and n_writers
respectively. The caller must create an i/o handler thread for each
segment in these arrays. This function also creates the sync array.
No i/o handler thread needs to be created for that
@param[in]	n_per_seg	maximum number of pending aio
                                operations allowed per segment
@param[in]	n_readers	number of reader threads
@param[in]	n_writers	number of writer threads
@param[in]	n_slots_sync	number of slots in the sync aio array
@return true if the AIO sub-system was started successfully */
bool AIO::start(ulint n_per_seg, ulint n_readers, ulint n_writers,
                ulint n_slots_sync) {
#if defined(LINUX_NATIVE_AIO)
  /* Check if native aio is supported on this system and tmpfs */
  if (srv_use_native_aio && !is_linux_native_aio_supported()) {
    ib::warn(ER_IB_MSG_829) << "Linux Native AIO disabled.";

    srv_use_native_aio = FALSE;
  }
#endif /* LINUX_NATIVE_AIO */

  srv_reset_io_thread_op_info();

  s_reads =
      create(LATCH_ID_OS_AIO_READ_MUTEX, n_readers * n_per_seg, n_readers);

  if (s_reads == NULL) {
    return (false);
  }

  ulint start = srv_read_only_mode ? 0 : 2;
  ulint n_segs = n_readers + start;

#ifndef UNIV_HOTBACKUP
  /* 0 is the ibuf segment and 1 is the redo log segment. */
  for (ulint i = start; i < n_segs; ++i) {
    ut_a(i < SRV_MAX_N_IO_THREADS);
    srv_io_thread_function[i] = "read thread";
  }
#endif /* !UNIV_HOTBACKUP */

  ulint n_segments = n_readers;

  if (!srv_read_only_mode) {
    s_ibuf = create(LATCH_ID_OS_AIO_IBUF_MUTEX, n_per_seg, 1);

    if (s_ibuf == NULL) {
      return (false);
    }

    ++n_segments;

#ifndef UNIV_HOTBACKUP
    srv_io_thread_function[0] = "insert buffer thread";
#endif /* !UNIV_HOTBACKUP */

    s_log = create(LATCH_ID_OS_AIO_LOG_MUTEX, n_per_seg, 1);

    if (s_log == NULL) {
      return (false);
    }

    ++n_segments;

#ifndef UNIV_HOTBACKUP
    srv_io_thread_function[1] = "log thread";
#endif /* !UNIV_HOTBAKUP */

  } else {
    s_ibuf = s_log = NULL;
  }

  s_writes =
      create(LATCH_ID_OS_AIO_WRITE_MUTEX, n_writers * n_per_seg, n_writers);

  if (s_writes == NULL) {
    return (false);
  }

  n_segments += n_writers;

#ifndef UNIV_HOTBACKUP
  for (ulint i = start + n_readers; i < n_segments; ++i) {
    ut_a(i < SRV_MAX_N_IO_THREADS);
    srv_io_thread_function[i] = "write thread";
  }
#endif /* !UNIV_HOTBACKUP */

  ut_ad(n_segments >= static_cast<ulint>(srv_read_only_mode ? 2 : 4));

  s_sync = create(LATCH_ID_OS_AIO_SYNC_MUTEX, n_slots_sync, 1);

  if (s_sync == NULL) {
    return (false);
  }

  os_aio_n_segments = n_segments;

  os_aio_validate();

  os_aio_segment_wait_events = static_cast<os_event_t *>(
      ut_zalloc_nokey(n_segments * sizeof *os_aio_segment_wait_events));

  if (os_aio_segment_wait_events == NULL) {
    return (false);
  }

  for (ulint i = 0; i < n_segments; ++i) {
    os_aio_segment_wait_events[i] = os_event_create(0);
  }

  os_last_printout = ut_time();

  return (true);
}



/** Generic AIO Handler methods. Currently handles IO post processing. */
class AIOHandler {
 public:
  /** Do any post processing after a read/write
  @return DB_SUCCESS or error code. */
  static dberr_t post_io_processing(Slot *slot);

  /** Decompress after a read and punch a hole in the file if
  it was a write */
  static dberr_t io_complete(const Slot *slot) {
    ut_a(slot->offset > 0);
    ut_a(slot->type.is_read() || !slot->skip_punch_hole);
    return (os_file_io_complete(slot->type, slot->file.m_file, slot->buf, NULL,
                                slot->original_len, slot->offset, slot->len));
  }

 private:
  /** Check whether the page was encrypted.
  @param[in]	slot		The slot that contains the IO request
  @return true if it was an encyrpted page */
  static bool is_encrypted_page(const Slot *slot) {
    return (Encryption::is_encrypted_page(slot->buf));
  }

  /** Check whether the page was compressed.
  @param[in]	slot		The slot that contains the IO request
  @return true if it was a compressed page */
  static bool is_compressed_page(const Slot *slot) {
    const byte *src = slot->buf;

    ulint page_type = mach_read_from_2(src + FIL_PAGE_TYPE);

    return (page_type == FIL_PAGE_COMPRESSED);
  }

  /** Get the compressed page size.
  @param[in]	slot		The slot that contains the IO request
  @return number of bytes to read for a successful decompress */
  static ulint compressed_page_size(const Slot *slot) {
    ut_ad(slot->type.is_read());
    ut_ad(is_compressed_page(slot));

    ulint size;
    const byte *src = slot->buf;

    size = mach_read_from_2(src + FIL_PAGE_COMPRESS_SIZE_V1);

    return (size + FIL_PAGE_DATA);
  }

  /** Check if the page contents can be decompressed.
  @param[in]	slot		The slot that contains the IO request
  @return true if the data read has all the compressed data */
  static bool can_decompress(const Slot *slot) {
    ut_ad(slot->type.is_read());
    ut_ad(is_compressed_page(slot));

    ulint version;
    const byte *src = slot->buf;

    version = mach_read_from_1(src + FIL_PAGE_VERSION);

    ut_a(version == 1);

    /* Includes the page header size too */
    ulint size = compressed_page_size(slot);

    return (size <= (slot->ptr - slot->buf) + (ulint)slot->n_bytes);
  }

  /** Check if we need to read some more data.
  @param[in]	slot		The slot that contains the IO request
  @param[in]	n_bytes		Total bytes read so far
  @return DB_SUCCESS or error code */
  static dberr_t check_read(Slot *slot, ulint n_bytes);
};





/** Check if we need to read some more data.
@param[in]	slot		The slot that contains the IO request
@param[in]	n_bytes		Total bytes read so far
@return DB_SUCCESS or error code */
dberr_t AIOHandler::check_read(Slot *slot, ulint n_bytes) {
  dberr_t err;

  ut_ad(slot->type.is_read());
  ut_ad(slot->original_len > slot->len);

  if (is_compressed_page(slot)) {
    if (can_decompress(slot)) {
      ut_a(slot->offset > 0);

      slot->len = slot->original_len;
#ifdef _WIN32
      slot->n_bytes = static_cast<DWORD>(n_bytes);
#else
      slot->n_bytes = static_cast<ulint>(n_bytes);
#endif /* _WIN32 */

      err = io_complete(slot);
      ut_a(err == DB_SUCCESS);
    } else {
      /* Read the next block in */
      ut_ad(compressed_page_size(slot) >= n_bytes);

      err = DB_FAIL;
    }
  } else if (is_encrypted_page(slot) ||
             (slot->type.is_log() && slot->offset >= LOG_FILE_HDR_SIZE)) {
    ut_a(slot->offset > 0);

    slot->len = slot->original_len;
#ifdef _WIN32
    slot->n_bytes = static_cast<DWORD>(n_bytes);
#else
    slot->n_bytes = static_cast<ulint>(n_bytes);
#endif /* _WIN32 */

    err = io_complete(slot);
    ut_a(err == DB_SUCCESS);

  } else {
    err = DB_FAIL;
  }

  if (slot->buf_block != NULL) {
    os_free_block(slot->buf_block);
    slot->buf_block = NULL;
  }

  if (slot->encrypt_log_buf != NULL) {
    ut_free(slot->encrypt_log_buf);
    slot->encrypt_log_buf = NULL;
  }

  return (err);
}

/** Do any post processing after a read/write
@return DB_SUCCESS or error code. */
dberr_t AIOHandler::post_io_processing(Slot *slot) {
  dberr_t err;

  ut_ad(slot->is_reserved);

  /* Total bytes read so far */
  ulint n_bytes = (slot->ptr - slot->buf) + slot->n_bytes;

  /* Compressed writes can be smaller than the original length.
  Therefore they can be processed without further IO. */
  if (n_bytes == slot->original_len ||
      (slot->type.is_write() && slot->type.is_compressed() &&
       slot->len == static_cast<ulint>(slot->n_bytes))) {
    if ((slot->type.is_log() && slot->offset >= LOG_FILE_HDR_SIZE) ||
        is_compressed_page(slot) || is_encrypted_page(slot)) {
      ut_a(slot->offset > 0);

      if (slot->type.is_read()) {
        slot->len = slot->original_len;
      }

      /* The punch hole has been done on collect() */

      if (slot->type.is_read()) {
        err = io_complete(slot);
      } else {
        err = DB_SUCCESS;
      }

      ut_ad(err == DB_SUCCESS || err == DB_UNSUPPORTED ||
            err == DB_CORRUPTION || err == DB_IO_DECOMPRESS_FAIL);
    } else {
      err = DB_SUCCESS;
    }

    if (slot->buf_block != NULL) {
      os_free_block(slot->buf_block);
      slot->buf_block = NULL;
    }

    if (slot->encrypt_log_buf != NULL) {
      ut_free(slot->encrypt_log_buf);
      slot->encrypt_log_buf = NULL;
    }
  } else if ((ulint)slot->n_bytes == (ulint)slot->len) {
    /* It *must* be a partial read. */
    ut_ad(slot->len < slot->original_len);

    /* Has to be a read request, if it is less than
    the original length. */
    ut_ad(slot->type.is_read());
    err = check_read(slot, n_bytes);

  } else {
    err = DB_FAIL;
  }

  return (err);
}


#ifdef UNIV_DEBUG
#ifndef UNIV_HOTBACKUP
/** Validates the consistency the aio system some of the time.
@return true if ok or the check was skipped */
static bool os_aio_validate_skip() {
/** Try os_aio_validate() every this many times */
#define OS_AIO_VALIDATE_SKIP 13

  /** The os_aio_validate() call skip counter.
  Use a signed type because of the race condition below. */
  static int os_aio_validate_count = OS_AIO_VALIDATE_SKIP;

  /* There is a race condition below, but it does not matter,
  because this call is only for heuristic purposes. We want to
  reduce the call frequency of the costly os_aio_validate()
  check in debug builds. */
  --os_aio_validate_count;

  if (os_aio_validate_count > 0) {
    return (true);
  }

  os_aio_validate_count = OS_AIO_VALIDATE_SKIP;
  return (os_aio_validate());
}
#endif /* !UNIV_HOTBACKUP */
#endif /* UNIV_DEBUG */



#ifndef UNIV_HOTBACKUP
/** Creates a temporary file.  This function is like tmpfile(3), but
the temporary file is created in the given parameter path. If the path
is NULL then it will create the file in the MySQL server configuration
parameter (--tmpdir).
@param[in]	path	location for creating temporary file
@return temporary file handle, or NULL on error */
FILE *os_file_create_tmpfile(const char *path) {
  FILE *file = NULL;
  int fd = innobase_mysql_tmpfile(path);

  if (fd >= 0) {
    file = fdopen(fd, "w+b");
  }

  if (file == NULL) {
    ib::error(ER_IB_MSG_751)
        << "Unable to create temporary file; errno: " << errno;

    if (fd >= 0) {
      close(fd);
    }
  }

  return (file);
}
#endif /* !UNIV_HOTBACKUP */

/** Rewind file to its start, read at most size - 1 bytes from it to str, and
NUL-terminate str. All errors are silently ignored. This function is
mostly meant to be used with temporary files.
@param[in,out]	file		File to read from
@param[in,out]	str		Buffer where to read
@param[in]	size		Size of buffer */
void os_file_read_string(FILE *file, char *str, ulint size) {
  if (size != 0) {
    rewind(file);

    size_t flen = fread(str, 1, size - 1, file);

    str[flen] = '\0';
  }
}



#ifdef UNIV_ENABLE_UNIT_TEST_GET_PARENT_DIR

/* Test the function os_file_get_parent_dir. */
void test_os_file_get_parent_dir(const char *child_dir,
                                 const char *expected_dir) {
  char *child = mem_strdup(child_dir);
  char *expected = expected_dir == NULL ? NULL : mem_strdup(expected_dir);

  /* os_file_get_parent_dir() assumes that separators are
  converted to OS_PATH_SEPARATOR. */
  Fil_path::normalize(child);
  Fil_path::normalize(expected);

  char *parent = os_file_get_parent_dir(child);

  bool unexpected =
      (expected == NULL ? (parent != NULL) : (0 != strcmp(parent, expected)));
  if (unexpected) {
    ib::fatal(ER_IB_MSG_752)
        << "os_file_get_parent_dir('" << child << "') returned '" << parent
        << "', instead of '" << expected << "'.";
  }
  ut_free(parent);
  ut_free(child);
  ut_free(expected);
}

/* Test the function os_file_get_parent_dir. */
void unit_test_os_file_get_parent_dir() {
  test_os_file_get_parent_dir("/usr/lib/a", "/usr/lib");
  test_os_file_get_parent_dir("/usr/", NULL);
  test_os_file_get_parent_dir("//usr//", NULL);
  test_os_file_get_parent_dir("usr", NULL);
  test_os_file_get_parent_dir("usr//", NULL);
  test_os_file_get_parent_dir("/", NULL);
  test_os_file_get_parent_dir("//", NULL);
  test_os_file_get_parent_dir(".", NULL);
  test_os_file_get_parent_dir("..", NULL);
#ifdef _WIN32
  test_os_file_get_parent_dir("D:", NULL);
  test_os_file_get_parent_dir("D:/", NULL);
  test_os_file_get_parent_dir("D:\\", NULL);
  test_os_file_get_parent_dir("D:/data", NULL);
  test_os_file_get_parent_dir("D:/data/", NULL);
  test_os_file_get_parent_dir("D:\\data\\", NULL);
  test_os_file_get_parent_dir("D:///data/////", NULL);
  test_os_file_get_parent_dir("D:\\\\\\data\\\\\\\\", NULL);
  test_os_file_get_parent_dir("D:/data//a", "D:/data");
  test_os_file_get_parent_dir("D:\\data\\\\a", "D:\\data");
  test_os_file_get_parent_dir("D:///data//a///b/", "D:///data//a");
  test_os_file_get_parent_dir("D:\\\\\\data\\\\a\\\\\\b\\",
                              "D:\\\\\\data\\\\a");
#endif /* _WIN32 */
}
#endif /* UNIV_ENABLE_UNIT_TEST_GET_PARENT_DIR */

#ifndef _WIN32




#if defined(LINUX_NATIVE_AIO)

/** Linux native AIO handler */
class LinuxAIOHandler {
 public:
  /**
  @param[in] global_segment	The global segment*/
  LinuxAIOHandler(ulint global_segment) : m_global_segment(global_segment) {
    /* Should never be doing Sync IO here. */
    ut_a(m_global_segment != ULINT_UNDEFINED);

    /* Find the array and the local segment. */

    m_segment = AIO::get_array_and_local_segment(&m_array, m_global_segment);

    m_n_slots = m_array->slots_per_segment();
  }

  /** Destructor */
  ~LinuxAIOHandler() {
    // No op
  }

  /**
  Process a Linux AIO request
  @param[out]	m1		the messages passed with the
  @param[out]	m2		AIO request; note that in case the
                                  AIO operation failed, these output
                                  parameters are valid and can be used to
                                  restart the operation.
  @param[out]	request		IO context
  @return DB_SUCCESS or error code */
  dberr_t poll(fil_node_t **m1, void **m2, IORequest *request);

 private:
  /** Resubmit an IO request that was only partially successful
  @param[in,out]	slot		Request to resubmit
  @return DB_SUCCESS or DB_FAIL if the IO resubmit request failed */
  dberr_t resubmit(Slot *slot);

  /** Check if the AIO succeeded
  @param[in,out]	slot		The slot to check
  @return DB_SUCCESS, DB_FAIL if the operation should be retried or
          DB_IO_ERROR on all other errors */
  dberr_t check_state(Slot *slot);

  /** @return true if a shutdown was detected */
  bool is_shutdown() const {
    return (srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS &&
            !buf_page_cleaner_is_active);
  }

  /** If no slot was found then the m_array->m_mutex will be released.
  @param[out]	n_pending	The number of pending IOs
  @return NULL or a slot that has completed IO */
  Slot *find_completed_slot(ulint *n_pending);

  /** This is called from within the IO-thread. If there are no completed
  IO requests in the slot array, the thread calls this function to
  collect more requests from the Linux kernel.
  The IO-thread waits on io_getevents(), which is a blocking call, with
  a timeout value. Unless the system is very heavy loaded, keeping the
  IO-thread very busy, the io-thread will spend most of its time waiting
  in this function.
  The IO-thread also exits in this function. It checks server status at
  each wakeup and that is why we use timed wait in io_getevents(). */
  void collect();

 private:
  /** Slot array */
  AIO *m_array;

  /** Number of slots inthe local segment */
  ulint m_n_slots;

  /** The local segment to check */
  ulint m_segment;

  /** The global segment */
  ulint m_global_segment;
};

/** Resubmit an IO request that was only partially successful
@param[in,out]	slot		Request to resubmit
@return DB_SUCCESS or DB_FAIL if the IO resubmit request failed */
dberr_t LinuxAIOHandler::resubmit(Slot *slot) {
#ifdef UNIV_DEBUG
  /* Bytes already read/written out */
  ulint n_bytes = slot->ptr - slot->buf;

  ut_ad(m_array->is_mutex_owned());

  ut_ad(n_bytes < slot->original_len);
  ut_ad(static_cast<ulint>(slot->n_bytes) < slot->original_len - n_bytes);
  /* Partial read or write scenario */
  ut_ad(slot->len >= static_cast<ulint>(slot->n_bytes));
#endif /* UNIV_DEBUG */

  slot->len -= slot->n_bytes;
  slot->ptr += slot->n_bytes;
  slot->offset += slot->n_bytes;

  /* Resetting the bytes read/written */
  slot->n_bytes = 0;
  slot->io_already_done = false;

  /* make sure that slot->offset fits in off_t */
  ut_ad(sizeof(off_t) >= sizeof(os_offset_t));
  struct iocb *iocb = &slot->control;
  if (slot->type.is_read()) {
    io_prep_pread(iocb, slot->file.m_file, slot->ptr, slot->len, slot->offset);

  } else {
    ut_a(slot->type.is_write());

    io_prep_pwrite(iocb, slot->file.m_file, slot->ptr, slot->len, slot->offset);
  }
  iocb->data = slot;

  /* Resubmit an I/O request */
  int ret = io_submit(m_array->io_ctx(m_segment), 1, &iocb);

  if (ret < -1) {
    errno = -ret;
  }

  return (ret < 0 ? DB_IO_PARTIAL_FAILED : DB_SUCCESS);
}

/** Check if the AIO succeeded
@param[in,out]	slot		The slot to check
@return DB_SUCCESS, DB_FAIL if the operation should be retried or
        DB_IO_ERROR on all other errors */
dberr_t LinuxAIOHandler::check_state(Slot *slot) {
  ut_ad(m_array->is_mutex_owned());

  /* Note that it may be that there is more then one completed
  IO requests. We process them one at a time. We may have a case
  here to improve the performance slightly by dealing with all
  requests in one sweep. */

  srv_set_io_thread_op_info(m_global_segment,
                            "processing completed aio requests");

  ut_ad(slot->io_already_done);

  dberr_t err;

  if (slot->ret == 0) {
    err = AIOHandler::post_io_processing(slot);

  } else {
    errno = -slot->ret;

    /* os_file_handle_error does tell us if we should retry
    this IO. As it stands now, we don't do this retry when
    reaping requests from a different context than
    the dispatcher. This non-retry logic is the same for
    Windows and Linux native AIO.
    We should probably look into this to transparently
    re-submit the IO. */
    os_file_handle_error(slot->name, "Linux aio");

    err = DB_IO_ERROR;
  }

  return (err);
}

/** If no slot was found then the m_array->m_mutex will be released.
@param[out]	n_pending		The number of pending IOs
@return NULL or a slot that has completed IO */
Slot *LinuxAIOHandler::find_completed_slot(ulint *n_pending) {
  ulint offset = m_n_slots * m_segment;

  *n_pending = 0;

  m_array->acquire();

  Slot *slot = m_array->at(offset);

  for (ulint i = 0; i < m_n_slots; ++i, ++slot) {
    if (slot->is_reserved) {
      ++*n_pending;

      if (slot->io_already_done) {
        /* Something for us to work on.
        Note: We don't release the mutex. */
        return (slot);
      }
    }
  }

  m_array->release();

  return (NULL);
}

/** This function is only used in Linux native asynchronous i/o. This is
called from within the io-thread. If there are no completed IO requests
in the slot array, the thread calls this function to collect more
requests from the kernel.
The io-thread waits on io_getevents(), which is a blocking call, with
a timeout value. Unless the system is very heavy loaded, keeping the
io-thread very busy, the io-thread will spend most of its time waiting
in this function.
The io-thread also exits in this function. It checks server status at
each wakeup and that is why we use timed wait in io_getevents(). */
void LinuxAIOHandler::collect() {
  ut_ad(m_n_slots > 0);
  ut_ad(m_array != NULL);
  ut_ad(m_segment < m_array->get_n_segments());

  /* Which io_context we are going to use. */
  io_context *io_ctx = m_array->io_ctx(m_segment);

  /* Starting point of the m_segment we will be working on. */
  ulint start_pos = m_segment * m_n_slots;

  /* End point. */
  ulint end_pos = start_pos + m_n_slots;

  for (;;) {
    struct io_event *events;

    /* Which part of event array we are going to work on. */
    events = m_array->io_events(m_segment * m_n_slots);

    /* Initialize the events. */
    memset(events, 0, sizeof(*events) * m_n_slots);

    /* The timeout value is arbitrary. We probably need
    to experiment with it a little. */
    struct timespec timeout;

    timeout.tv_sec = 0;
    timeout.tv_nsec = OS_AIO_REAP_TIMEOUT;

    int ret;

    ret = io_getevents(io_ctx, 1, m_n_slots, events, &timeout);

    for (int i = 0; i < ret; ++i) {
      struct iocb *iocb;

      iocb = reinterpret_cast<struct iocb *>(events[i].obj);
      ut_a(iocb != NULL);

      Slot *slot = reinterpret_cast<Slot *>(iocb->data);

      /* Some sanity checks. */
      ut_a(slot != NULL);
      ut_a(slot->is_reserved);

      /* We are not scribbling previous segment. */
      ut_a(slot->pos >= start_pos);

      /* We have not overstepped to next segment. */
      ut_a(slot->pos < end_pos);

      /* We never compress/decompress the first page */

      if (slot->offset > 0 && !slot->skip_punch_hole &&
          slot->type.is_compression_enabled() && !slot->type.is_log() &&
          slot->type.is_write() && slot->type.is_compressed() &&
          slot->type.punch_hole()) {
        slot->err = AIOHandler::io_complete(slot);
      } else {
        slot->err = DB_SUCCESS;
      }

      /* Mark this request as completed. The error handling
      will be done in the calling function. */
      m_array->acquire();

      /* events[i].res2 should always be ZERO */
      ut_ad(events[i].res2 == 0);
      slot->io_already_done = true;

      /*Even though events[i].res is an unsigned number in libaio, it is
      used to return a negative value (negated errno value) to indicate
      error and a positive value to indicate number of bytes read or
      written. */

      if (events[i].res > slot->len) {
        /* failure */
        slot->n_bytes = 0;
        slot->ret = events[i].res;
      } else {
        /* success */
        slot->n_bytes = events[i].res;
        slot->ret = 0;
      }
      m_array->release();
    }

    if (srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS ||
        !buf_page_cleaner_is_active || ret > 0) {
      break;
    }

    /* This error handling is for any error in collecting the
    IO requests. The errors, if any, for any particular IO
    request are simply passed on to the calling routine. */

    switch (ret) {
      case -EAGAIN:
        /* Not enough resources! Try again. */

      case -EINTR:
        /* Interrupted! The behaviour in case of an interrupt.
        If we have some completed IOs available then the
        return code will be the number of IOs. We get EINTR
        only if there are no completed IOs and we have been
        interrupted. */

      case 0:
        /* No pending request! Go back and check again. */

        continue;
    }

    /* All other errors should cause a trap for now. */
    ib::fatal(ER_IB_MSG_755)
        << "Unexpected ret_code[" << ret << "] from io_getevents()!";

    break;
  }
}

/** Process a Linux AIO request
@param[out]	m1		the messages passed with the
@param[out]	m2		AIO request; note that in case the
                                AIO operation failed, these output
                                parameters are valid and can be used to
                                restart the operation.
@param[out]	request		IO context
@return DB_SUCCESS or error code */
dberr_t LinuxAIOHandler::poll(fil_node_t **m1, void **m2, IORequest *request) {
  dberr_t err;
  Slot *slot;

  /* Loop until we have found a completed request. */
  for (;;) {
    ulint n_pending;

    slot = find_completed_slot(&n_pending);

    if (slot != NULL) {
      ut_ad(m_array->is_mutex_owned());

      err = check_state(slot);

      /* DB_FAIL is not a hard error, we should retry */
      if (err != DB_FAIL) {
        break;
      }

      /* Partial IO, resubmit request for
      remaining bytes to read/write */
      err = resubmit(slot);

      if (err != DB_SUCCESS) {
        break;
      }

      m_array->release();

    } else if (is_shutdown() && n_pending == 0) {
      /* There is no completed request. If there is
      no pending request at all, and the system is
      being shut down, exit. */

      *m1 = NULL;
      *m2 = NULL;

      return (DB_SUCCESS);

    } else {
      /* Wait for some request. Note that we return
      from wait if we have found a request. */

      srv_set_io_thread_op_info(m_global_segment,
                                "waiting for completed aio requests");

      collect();
    }
  }

  if (err == DB_IO_PARTIAL_FAILED) {
    /* Aborting in case of submit failure */
    ib::fatal(ER_IB_MSG_756) << "Native Linux AIO interface. "
                                "io_submit() call failed when "
                                "resubmitting a partial I/O "
                                "request on the file "
                             << slot->name << ".";
  }

  *m1 = slot->m1;
  *m2 = slot->m2;

  *request = slot->type;

  m_array->release(slot);

  m_array->release();

  return (err);
}

/** This function is only used in Linux native asynchronous i/o.
Waits for an aio operation to complete. This function is used to wait for
the completed requests. The aio array of pending requests is divided
into segments. The thread specifies which segment or slot it wants to wait
for. NOTE: this function will also take care of freeing the aio slot,
therefore no other thread is allowed to do the freeing!

@param[in]	global_segment	segment number in the aio array
                                to wait for; segment 0 is the ibuf
                                i/o thread, segment 1 is log i/o thread,
                                then follow the non-ibuf read threads,
                                and the last are the non-ibuf write
                                threads.
@param[out]	m1		the messages passed with the
@param[out]	m2			AIO request; note that in case the
                                AIO operation failed, these output
                                parameters are valid and can be used to
                                restart the operation.
@param[out]	request		IO context
@return DB_SUCCESS if the IO was successful */
static dberr_t os_aio_linux_handler(ulint global_segment, fil_node_t **m1,
                                    void **m2, IORequest *request) {
  LinuxAIOHandler handler(global_segment);

  dberr_t err = handler.poll(m1, m2, request);

  if (err == DB_IO_NO_PUNCH_HOLE) {
    fil_no_punch_hole(*m1);
    err = DB_SUCCESS;
  }

  return (err);
}

#endif /* LINUX_NATIVE_AIO */



















#ifdef UNIV_HOTBACKUP
/** Closes a file handle.
@param[in]	file		Handle to a file
@return true if success */
bool os_file_close_no_error_handling(os_file_t file) {
  return (close(file) != -1);
}
#endif /* UNIV_HOTBACKUP */

/** This function can be called if one wants to post a batch of reads and
prefers an i/o-handler thread to handle them all at once later. You must
call os_aio_simulated_wake_handler_threads later to ensure the threads
are not left sleeping! */
void os_aio_simulated_put_read_threads_to_sleep() { /* No op on non Windows */
}

#else /* !_WIN32 */

#include <WinIoCtl.h>

/** Do the read/write
@param[in]	request	The IO context and type
@return the number of bytes read/written or negative value on error */
ssize_t SyncFileIO::execute(const IORequest &request) {
  OVERLAPPED seek;

  memset(&seek, 0x0, sizeof(seek));

  seek.Offset = (DWORD)m_offset & 0xFFFFFFFF;
  seek.OffsetHigh = (DWORD)(m_offset >> 32);

  BOOL ret;
  DWORD n_bytes;

  if (request.is_read()) {
    ret = ReadFile(m_fh, m_buf, static_cast<DWORD>(m_n), &n_bytes, &seek);

  } else {
    ut_ad(request.is_write());
    ret = WriteFile(m_fh, m_buf, static_cast<DWORD>(m_n), &n_bytes, &seek);
  }

  return (ret ? static_cast<ssize_t>(n_bytes) : -1);
}

/** Do the read/write
@param[in,out]	slot	The IO slot, it has the IO context
@return the number of bytes read/written or negative value on error */
ssize_t SyncFileIO::execute(Slot *slot) {
  BOOL ret;

  if (slot->type.is_read()) {
    ret = ReadFile(slot->file.m_file, slot->ptr, slot->len, &slot->n_bytes,
                   &slot->control);
  } else {
    ut_ad(slot->type.is_write());
    ret = WriteFile(slot->file.m_file, slot->ptr, slot->len, &slot->n_bytes,
                    &slot->control);
  }

  return (ret ? static_cast<ssize_t>(slot->n_bytes) : -1);
}

/** Check if the file system supports sparse files.
@param[in]	 name		File name
@return true if the file system supports sparse files */
static bool os_is_sparse_file_supported_win32(const char *filename) {
  char volname[MAX_PATH];
  BOOL result = GetVolumePathName(filename, volname, MAX_PATH);

  if (!result) {
    ib::error(ER_IB_MSG_785)
        << "os_is_sparse_file_supported: "
        << "Failed to get the volume path name for: " << filename
        << "- OS error number " << GetLastError();

    return (false);
  }

  DWORD flags;

  GetVolumeInformation(volname, NULL, MAX_PATH, NULL, NULL, &flags, NULL,
                       MAX_PATH);

  return (flags & FILE_SUPPORTS_SPARSE_FILES) ? true : false;
}

/** Free storage space associated with a section of the file.
@param[in]	fh		Open file handle
@param[in]	page_size	Tablespace page size
@param[in]	block_size	File system block size
@param[in]	off		Starting offset (SEEK_SET)
@param[in]	len		Size of the hole
@return 0 on success or errno */
static dberr_t os_file_punch_hole_win32(os_file_t fh, os_offset_t off,
                                        os_offset_t len) {
  FILE_ZERO_DATA_INFORMATION punch;

  punch.FileOffset.QuadPart = off;
  punch.BeyondFinalZero.QuadPart = off + len;

  /* If lpOverlapped is NULL, lpBytesReturned cannot be NULL,
  therefore we pass a dummy parameter. */
  DWORD temp;

  BOOL result = DeviceIoControl(fh, FSCTL_SET_ZERO_DATA, &punch, sizeof(punch),
                                NULL, 0, &temp, NULL);

  return (!result ? DB_IO_NO_PUNCH_HOLE : DB_SUCCESS);
}

/** Check the existence and type of the given file.
@param[in]	path		path name of file
@param[out]	exists		true if the file exists
@param[out]	type		Type of the file, if it exists
@return true if call succeeded */
static bool os_file_status_win32(const char *path, bool *exists,
                                 os_file_type_t *type) {
  struct _stat64 statinfo;

  int ret = _stat64(path, &statinfo);

  if (ret == 0) {
    /* file exists, everything OK */

  } else if (errno == ENOENT || errno == ENOTDIR) {
    *type = OS_FILE_TYPE_MISSING;

    /* file does not exist */

    if (exists != nullptr) {
      *exists = false;
    }

    return (true);

  } else if (errno == EACCES) {
    *type = OS_FILE_PERMISSION_ERROR;
    return (false);

  } else {
    *type = OS_FILE_TYPE_FAILED;

    /* file exists, but stat call failed */
    os_file_handle_error_no_exit(path, "stat", false);
    return (false);
  }

  if (exists != nullptr) {
    *exists = true;
  }

  if (_S_IFDIR & statinfo.st_mode) {
    *type = OS_FILE_TYPE_DIR;

  } else if (_S_IFREG & statinfo.st_mode) {
    *type = OS_FILE_TYPE_FILE;

  } else {
    *type = OS_FILE_TYPE_UNKNOWN;
  }

  return (true);
}

/** NOTE! Use the corresponding macro os_file_flush(), not directly this
function!
Flushes the write buffers of a given file to the disk.
@param[in]	file		handle to a file
@return true if success */
bool os_file_flush_func(os_file_t file) {
  ++os_n_fsyncs;

  BOOL ret = FlushFileBuffers(file);

  if (ret) {
    return (true);
  }

  /* Since Windows returns ERROR_INVALID_FUNCTION if the 'file' is
  actually a raw device, we choose to ignore that error if we are using
  raw disks */

  if (srv_start_raw_disk_in_use && GetLastError() == ERROR_INVALID_FUNCTION) {
    return (true);
  }

  os_file_handle_error(NULL, "flush");

  /* It is a fatal error if a file flush does not succeed, because then
  the database can get corrupt on disk */
  ut_error;
}

/** Retrieves the last error number if an error occurs in a file io function.
The number should be retrieved before any other OS calls (because they may
overwrite the error number). If the number is not known to this program,
the OS error number + 100 is returned.
@param[in]	report_all_errors	true if we want an error message printed
                                        of all errors
@param[in]	on_error_silent		true then don't print any diagnostic
                                        to the log
@return error number, or OS error number + 100 */
static ulint os_file_get_last_error_low(bool report_all_errors,
                                        bool on_error_silent) {
  ulint err = (ulint)GetLastError();

  if (err == ERROR_SUCCESS) {
    return (0);
  }

  if (report_all_errors || (!on_error_silent && err != ERROR_DISK_FULL &&
                            err != ERROR_FILE_EXISTS)) {
    ib::error(ER_IB_MSG_786)
        << "Operating system error number " << err << " in a file operation.";

    if (err == ERROR_PATH_NOT_FOUND) {
      ib::error(ER_IB_MSG_787) << "The error means the system"
                                  " cannot find the path specified.";

#ifndef UNIV_HOTBACKUP
      if (srv_is_being_started) {
        ib::error(ER_IB_MSG_788) << "If you are installing InnoDB,"
                                    " remember that you must create"
                                    " directories yourself, InnoDB"
                                    " does not create them.";
      }
#endif /* !UNIV_HOTBACKUP */

    } else if (err == ERROR_ACCESS_DENIED) {
      ib::error(ER_IB_MSG_789) << "The error means mysqld does not have"
                                  " the access rights to"
                                  " the directory. It may also be"
                                  " you have created a subdirectory"
                                  " of the same name as a data file.";

    } else if (err == ERROR_SHARING_VIOLATION || err == ERROR_LOCK_VIOLATION) {
      ib::error(ER_IB_MSG_790) << "The error means that another program"
                                  " is using InnoDB's files."
                                  " This might be a backup or antivirus"
                                  " software or another instance"
                                  " of MySQL."
                                  " Please close it to get rid of this error.";

    } else if (err == ERROR_WORKING_SET_QUOTA ||
               err == ERROR_NO_SYSTEM_RESOURCES) {
      ib::error(ER_IB_MSG_791) << "The error means that there are no"
                                  " sufficient system resources or quota to"
                                  " complete the operation.";

    } else if (err == ERROR_OPERATION_ABORTED) {
      ib::error(ER_IB_MSG_792) << "The error means that the I/O"
                                  " operation has been aborted"
                                  " because of either a thread exit"
                                  " or an application request."
                                  " Retry attempt is made.";
    } else {
      ib::info(ER_IB_MSG_793) << OPERATING_SYSTEM_ERROR_MSG;
    }
  }

  if (err == ERROR_FILE_NOT_FOUND) {
    return (OS_FILE_NOT_FOUND);
  } else if (err == ERROR_DISK_FULL) {
    return (OS_FILE_DISK_FULL);
  } else if (err == ERROR_FILE_EXISTS) {
    return (OS_FILE_ALREADY_EXISTS);
  } else if (err == ERROR_SHARING_VIOLATION || err == ERROR_LOCK_VIOLATION) {
    return (OS_FILE_SHARING_VIOLATION);
  } else if (err == ERROR_WORKING_SET_QUOTA ||
             err == ERROR_NO_SYSTEM_RESOURCES) {
    return (OS_FILE_INSUFFICIENT_RESOURCE);
  } else if (err == ERROR_OPERATION_ABORTED) {
    return (OS_FILE_OPERATION_ABORTED);
  } else if (err == ERROR_ACCESS_DENIED) {
    return (OS_FILE_ACCESS_VIOLATION);
  }

  return (OS_FILE_ERROR_MAX + err);
}

/** NOTE! Use the corresponding macro os_file_create_simple(), not directly
this function!
A simple function to open or create a file.
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	access_type	OS_FILE_READ_ONLY or OS_FILE_READ_WRITE
@param[in]	read_only	if true read only mode checks are enforced
@param[out]	success		true if succeed, false if error
@return handle to the file, not defined if error, error number
        can be retrieved with os_file_get_last_error */
os_file_t os_file_create_simple_func(const char *name, ulint create_mode,
                                     ulint access_type, bool read_only,
                                     bool *success) {
  os_file_t file;

  *success = false;

  DWORD access;
  DWORD create_flag;
  DWORD attributes = 0;
  DWORD share_mode = FILE_SHARE_READ;

  ut_a(!(create_mode & OS_FILE_ON_ERROR_SILENT));
  ut_a(!(create_mode & OS_FILE_ON_ERROR_NO_EXIT));

  if (create_mode == OS_FILE_OPEN) {
    create_flag = OPEN_EXISTING;

  } else if (read_only) {
    create_flag = OPEN_EXISTING;

  } else if (create_mode == OS_FILE_CREATE) {
    create_flag = CREATE_NEW;

  } else if (create_mode == OS_FILE_CREATE_PATH) {
    /* Create subdirs along the path if needed. */
    dberr_t err;

    err = os_file_create_subdirs_if_needed(name);

    if (err != DB_SUCCESS) {
      *success = false;
      ib::error(ER_IB_MSG_794)
          << "Unable to create subdirectories '" << name << "'";

      return (OS_FILE_CLOSED);
    }

    create_flag = CREATE_NEW;
    create_mode = OS_FILE_CREATE;

  } else {
    ib::error(ER_IB_MSG_795) << "Unknown file create mode (" << create_mode
                             << ") for file '" << name << "'";

    return (OS_FILE_CLOSED);
  }

  if (access_type == OS_FILE_READ_ONLY) {
    access = GENERIC_READ;

  } else if (access_type == OS_FILE_READ_ALLOW_DELETE) {
    ut_ad(read_only);

    access = GENERIC_READ;
    share_mode |= FILE_SHARE_DELETE | FILE_SHARE_WRITE;

  } else if (read_only) {
    ib::info(ER_IB_MSG_796) << "Read only mode set. Unable to"
                               " open file '"
                            << name << "' in RW mode, "
                            << "trying RO mode",
        name;

    access = GENERIC_READ;

  } else if (access_type == OS_FILE_READ_WRITE) {
    access = GENERIC_READ | GENERIC_WRITE;

  } else {
    ib::error(ER_IB_MSG_797) << "Unknown file access type (" << access_type
                             << ") "
                                "for file '"
                             << name << "'";

    return (OS_FILE_CLOSED);
  }

  bool retry;

  do {
    /* Use default security attributes and no template file. */

    file = CreateFile((LPCTSTR)name, access, share_mode, NULL, create_flag,
                      attributes, NULL);

    if (file == INVALID_HANDLE_VALUE) {
      *success = false;

      retry = os_file_handle_error(
          name, create_mode == OS_FILE_OPEN ? "open" : "create");

    } else {
      retry = false;

      *success = true;

      DWORD temp;

      /* This is a best effort use case, if it fails then
      we will find out when we try and punch the hole. */

      DeviceIoControl(file, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &temp, NULL);
    }

  } while (retry);

  return (file);
}

/** This function attempts to create a directory named pathname. The new
directory gets default permissions. On Unix the permissions are
(0770 & ~umask). If the directory exists already, nothing is done and
the call succeeds, unless the fail_if_exists arguments is true.
If another error occurs, such as a permission error, this does not crash,
but reports the error and returns false.
@param[in]	pathname	directory name as null-terminated string
@param[in]	fail_if_exists	if true, pre-existing directory is treated
                                as an error.
@return true if call succeeds, false on error */
bool os_file_create_directory(const char *pathname, bool fail_if_exists) {
  BOOL rcode;

  rcode = CreateDirectory((LPCTSTR)pathname, NULL);
  if (!(rcode != 0 ||
        (GetLastError() == ERROR_ALREADY_EXISTS && !fail_if_exists))) {
    os_file_handle_error_no_exit(pathname, "CreateDirectory", false);

    return (false);
  }

  return (true);
}

/** This function scans the contents of a directory and invokes the callback
for each entry.
@param[in]	path		directory name as null-terminated string
@param[in]	scan_cbk	use callback to be called for each entry
@param[in]	is_drop		attempt to drop the directory after scan
@return true if call succeeds, false on error */
bool os_file_scan_directory(const char *path, os_dir_cbk_t scan_cbk,
                            bool is_drop) {
  bool file_found;
  HANDLE find_hdl;
  WIN32_FIND_DATA find_data;
  char wild_card_path[MAX_PATH];

  snprintf(wild_card_path, MAX_PATH, "%s\\*", path);

  find_hdl = FindFirstFile((LPCTSTR)wild_card_path, &find_data);

  if (find_hdl == INVALID_HANDLE_VALUE) {
    os_file_handle_error_no_exit(path, "FindFirstFile", false);
    return (false);
  }

  do {
    scan_cbk(path, find_data.cFileName);
    file_found = FindNextFile(find_hdl, &find_data);

  } while (file_found);

  FindClose(find_hdl);

  if (is_drop) {
    bool ret;

    ret = RemoveDirectory((LPCSTR)path);

    if (!ret) {
      os_file_handle_error_no_exit(path, "RemoveDirectory", false);
      return (false);
    }
  }

  return (true);
}

/** NOTE! Use the corresponding macro os_file_create(), not directly
this function!
Opens an existing file or creates a new.
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	purpose		OS_FILE_AIO, if asynchronous, non-buffered I/O
                                is desired, OS_FILE_NORMAL, if any normal file;
                                NOTE that it also depends on type, os_aio_..
                                and srv_.. variables whether we really use async
                                I/O or unbuffered I/O: look in the function
                                source code for the exact rules
@param[in]	type		OS_DATA_FILE or OS_LOG_FILE
@param[in]	success		true if succeeded
@return handle to the file, not defined if error, error number
        can be retrieved with os_file_get_last_error */
pfs_os_file_t os_file_create_func(const char *name, ulint create_mode,
                                  ulint purpose, ulint type, bool read_only,
                                  bool *success) {
  pfs_os_file_t file;
  bool retry;
  bool on_error_no_exit;
  bool on_error_silent;

  *success = false;

  DBUG_EXECUTE_IF("ib_create_table_fail_disk_full", *success = false;
                  SetLastError(ERROR_DISK_FULL); file.m_file = OS_FILE_CLOSED;
                  return (file););

  DWORD create_flag;
  DWORD share_mode = FILE_SHARE_READ;

  on_error_no_exit = create_mode & OS_FILE_ON_ERROR_NO_EXIT ? true : false;

  on_error_silent = create_mode & OS_FILE_ON_ERROR_SILENT ? true : false;

  create_mode &= ~OS_FILE_ON_ERROR_NO_EXIT;
  create_mode &= ~OS_FILE_ON_ERROR_SILENT;

  if (create_mode == OS_FILE_OPEN_RAW) {
    ut_a(!read_only);

    create_flag = OPEN_EXISTING;

    /* On Windows Physical devices require admin privileges and
    have to have the write-share mode set. See the remarks
    section for the CreateFile() function documentation in MSDN. */

    share_mode |= FILE_SHARE_WRITE;

  } else if (create_mode == OS_FILE_OPEN || create_mode == OS_FILE_OPEN_RETRY) {
    create_flag = OPEN_EXISTING;

  } else if (read_only) {
    create_flag = OPEN_EXISTING;

  } else if (create_mode == OS_FILE_CREATE) {
    create_flag = CREATE_NEW;

  } else if (create_mode == OS_FILE_CREATE_PATH) {
    /* Create subdirs along the path if needed. */
    dberr_t err;

    err = os_file_create_subdirs_if_needed(name);

    if (err != DB_SUCCESS) {
      *success = false;
      ib::error(ER_IB_MSG_798)
          << "Unable to create subdirectories '" << name << "'";

      file.m_file = OS_FILE_CLOSED;
      return (file);
    }

    create_flag = CREATE_NEW;
    create_mode = OS_FILE_CREATE;

  } else {
    ib::error(ER_IB_MSG_799)
        << "Unknown file create mode (" << create_mode << ") "
        << " for file '" << name << "'";

    file.m_file = OS_FILE_CLOSED;
    return (file);
  }

  DWORD attributes = 0;

#ifdef UNIV_HOTBACKUP
  attributes |= FILE_FLAG_NO_BUFFERING;
#else /* UNIV_HOTBACKUP */
  if (purpose == OS_FILE_AIO) {

#ifdef WIN_ASYNC_IO
    /* If specified, use asynchronous (overlapped) io and no
    buffering of writes in the OS */

    if (srv_use_native_aio) {
      attributes |= FILE_FLAG_OVERLAPPED;
    }
#endif /* WIN_ASYNC_IO */

  } else if (purpose == OS_FILE_NORMAL) {
    /* Use default setting. */

  } else {
    ib::error(ER_IB_MSG_800) << "Unknown purpose flag (" << purpose << ") "
                             << "while opening file '" << name << "'";

    file.m_file = OS_FILE_CLOSED;
    return (file);
  }

#ifdef UNIV_NON_BUFFERED_IO
  // TODO: Create a bug, this looks wrong. The flush log
  // parameter is dynamic.
  if ((type == OS_BUFFERED_FILE) || (type == OS_CLONE_LOG_FILE) ||
      (type == OS_LOG_FILE)) {
    /* Do not use unbuffered i/o for the log files because
    we write really a lot and we have log flusher for fsyncs. */

  } else if (srv_win_file_flush_method == SRV_WIN_IO_UNBUFFERED) {
    attributes |= FILE_FLAG_NO_BUFFERING;
  }
#endif /* UNIV_NON_BUFFERED_IO */

#endif /* UNIV_HOTBACKUP */
  DWORD access = GENERIC_READ;

  if (!read_only) {
    access |= GENERIC_WRITE;
  }

  /* Clone must allow concurrent write to file. */
  if (type == OS_CLONE_LOG_FILE || type == OS_CLONE_DATA_FILE) {
    share_mode |= FILE_SHARE_WRITE;
  }

  do {
    /* Use default security attributes and no template file. */
    file.m_file = CreateFile((LPCTSTR)name, access, share_mode, NULL,
                             create_flag, attributes, NULL);

    if (file.m_file == INVALID_HANDLE_VALUE) {
      const char *operation;

      operation =
          (create_mode == OS_FILE_CREATE && !read_only) ? "create" : "open";

      *success = false;

      if (on_error_no_exit) {
        retry = os_file_handle_error_no_exit(name, operation, on_error_silent);
      } else {
        retry = os_file_handle_error(name, operation);
      }
    } else {
      retry = false;

      *success = true;

      DWORD temp;

      /* This is a best effort use case, if it fails then
      we will find out when we try and punch the hole. */
      DeviceIoControl(file.m_file, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &temp,
                      NULL);
    }

  } while (retry);

  return (file);
}

/** NOTE! Use the corresponding macro os_file_create_simple_no_error_handling(),
not directly this function!
A simple function to open or create a file.
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	access_type	OS_FILE_READ_ONLY, OS_FILE_READ_WRITE, or
                                OS_FILE_READ_ALLOW_DELETE; the last option is
                                used by a backup program reading the file
@param[out]	success		true if succeeded
@return own: handle to the file, not defined if error, error number
        can be retrieved with os_file_get_last_error */
pfs_os_file_t os_file_create_simple_no_error_handling_func(const char *name,
                                                           ulint create_mode,
                                                           ulint access_type,
                                                           bool read_only,
                                                           bool *success) {
  pfs_os_file_t file;

  *success = false;

  DWORD access;
  DWORD create_flag;
  DWORD attributes = 0;
  DWORD share_mode = FILE_SHARE_READ;

#ifdef UNIV_HOTBACKUP
  share_mode |= FILE_SHARE_WRITE;
#endif /* UNIV_HOTBACKUP */

  ut_a(name);

  ut_a(!(create_mode & OS_FILE_ON_ERROR_SILENT));
  ut_a(!(create_mode & OS_FILE_ON_ERROR_NO_EXIT));

  if (create_mode == OS_FILE_OPEN) {
    create_flag = OPEN_EXISTING;

  } else if (read_only) {
    create_flag = OPEN_EXISTING;

  } else if (create_mode == OS_FILE_CREATE) {
    create_flag = CREATE_NEW;

  } else {
    ib::error(ER_IB_MSG_801)
        << "Unknown file create mode (" << create_mode << ") "
        << " for file '" << name << "'";

    file.m_file = OS_FILE_CLOSED;
    return (file);
  }

  if (access_type == OS_FILE_READ_ONLY) {
    access = GENERIC_READ;

  } else if (read_only) {
    access = GENERIC_READ;

  } else if (access_type == OS_FILE_READ_WRITE) {
    access = GENERIC_READ | GENERIC_WRITE;

  } else if (access_type == OS_FILE_READ_ALLOW_DELETE) {
    ut_a(!read_only);

    access = GENERIC_READ;

    /* A backup program has to give mysqld the maximum
    freedom to do what it likes with the file */

    share_mode |= FILE_SHARE_DELETE | FILE_SHARE_WRITE;
  } else {
    ib::error(ER_IB_MSG_802)
        << "Unknown file access type (" << access_type << ") "
        << "for file '" << name << "'";

    file.m_file = OS_FILE_CLOSED;
    return (file);
  }

  file.m_file = CreateFile((LPCTSTR)name, access, share_mode,
                           NULL,  // Security attributes
                           create_flag, attributes,
                           NULL);  // No template file

  *success = (file.m_file != INVALID_HANDLE_VALUE);

  return (file);
}

/** Deletes a file if it exists. The file has to be closed before calling this.
@param[in]	name		file path as a null-terminated string
@param[out]	exist		indicate if file pre-exist
@return true if success */
bool os_file_delete_if_exists_func(const char *name, bool *exist) {
  if (!os_file_can_delete(name)) {
    return (false);
  }

  if (exist != nullptr) {
    *exist = true;
  }

  ulint count = 0;

  for (;;) {
    /* In Windows, deleting an .ibd file may fail if mysqlbackup
    is copying it */

    bool ret = DeleteFile((LPCTSTR)name);

    if (ret) {
      return (true);
    }

    DWORD lasterr = GetLastError();

    if (lasterr == ERROR_FILE_NOT_FOUND || lasterr == ERROR_PATH_NOT_FOUND) {
      /* The file does not exist, this not an error */
      if (exist != NULL) {
        *exist = false;
      }

      return (true);
    }

    ++count;

    if (count > 100 && 0 == (count % 10)) {
      /* Print error information */
      os_file_get_last_error(true);

      ib::warn(ER_IB_MSG_803) << "Delete of file '" << name << "' failed.";
    }

    /* Sleep for a second */
    os_thread_sleep(1000000);

    if (count > 2000) {
      return (false);
    }
  }
}

/** Deletes a file. The file has to be closed before calling this.
@param[in]	name		File path as NUL terminated string
@return true if success */
bool os_file_delete_func(const char *name) {
  ulint count = 0;

  for (;;) {
    /* In Windows, deleting an .ibd file may fail if mysqlbackup
    is copying it */

    BOOL ret = DeleteFile((LPCTSTR)name);

    if (ret) {
      return (true);
    }

    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
      /* If the file does not exist, we classify this as
      a 'mild' error and return */

      return (false);
    }

    ++count;

    if (count > 100 && 0 == (count % 10)) {
      /* print error information */
      os_file_get_last_error(true);

      ib::warn(ER_IB_MSG_804)
          << "Cannot delete file '" << name << "'. Are you running mysqlbackup"
          << " to back up the file?";
    }

    /* sleep for a second */
    os_thread_sleep(1000000);

    if (count > 2000) {
      return (false);
    }
  }

  ut_error;
  return (false);
}

/** NOTE! Use the corresponding macro os_file_rename(), not directly this
function!
Renames a file (can also move it to another directory). It is safest that the
file is closed before calling this function.
@param[in]	oldpath		old file path as a null-terminated string
@param[in]	newpath		new file path
@return true if success */
bool os_file_rename_func(const char *oldpath, const char *newpath) {
#ifdef UNIV_DEBUG
  os_file_type_t type;
  bool exists;

  /* New path must not exist. */
  ut_ad(os_file_status(newpath, &exists, &type));
  ut_ad(!exists);

  /* Old path must exist. */
  ut_ad(os_file_status(oldpath, &exists, &type));
  ut_ad(exists);
#endif /* UNIV_DEBUG */

  if (MoveFile((LPCTSTR)oldpath, (LPCTSTR)newpath)) {
    return (true);
  }

  os_file_handle_error_no_exit(oldpath, "rename", false);

  return (false);
}

/** NOTE! Use the corresponding macro os_file_close(), not directly
this function!
Closes a file handle. In case of error, error number can be retrieved with
os_file_get_last_error.
@param[in,own]	file		Handle to a file
@return true if success */
bool os_file_close_func(os_file_t file) {
  ut_a(file != INVALID_HANDLE_VALUE);

  if (CloseHandle(file)) {
    return (true);
  }

  os_file_handle_error(NULL, "close");

  return (false);
}

/** Gets a file size.
@param[in]	file		Handle to a file
@return file size, or (os_offset_t) -1 on failure */
os_offset_t os_file_get_size(pfs_os_file_t file) {
  DWORD high;
  DWORD low;

  low = GetFileSize(file.m_file, &high);
  if (low == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
    return ((os_offset_t)-1);
  }

  return (os_offset_t(low | (os_offset_t(high) << 32)));
}

/** Gets a file size.
@param[in]	filename	Full path to the filename to check
@return file size if OK, else set m_total_size to ~0 and m_alloc_size to
        errno */
os_file_size_t os_file_get_size(const char *filename) {
  struct __stat64 s;
  os_file_size_t file_size;

  int ret = _stat64(filename, &s);

  if (ret == 0) {
    file_size.m_total_size = s.st_size;

    DWORD low_size;
    DWORD high_size;

    low_size = GetCompressedFileSize(filename, &high_size);

    if (low_size != INVALID_FILE_SIZE) {
      file_size.m_alloc_size = high_size;
      file_size.m_alloc_size <<= 32;
      file_size.m_alloc_size |= low_size;

    } else {
      ib::error(ER_IB_MSG_805)
          << "GetCompressedFileSize(" << filename << ", ..) failed.";

      file_size.m_alloc_size = (os_offset_t)-1;
    }
  } else {
    file_size.m_total_size = ~0;
    file_size.m_alloc_size = (os_offset_t)ret;
  }

  return (file_size);
}

/** This function returns information about the specified file
@param[in]	path		pathname of the file
@param[out]	stat_info	information of a file in a directory
@param[in,out]	statinfo	information of a file in a directory
@param[in]	check_rw_perm	for testing whether the file can be opened
                                in RW mode
@param[in]	read_only	true if the file is opened in read-only mode
@return DB_SUCCESS if all OK */
static dberr_t os_file_get_status_win32(const char *path,
                                        os_file_stat_t *stat_info,
                                        struct _stat64 *statinfo,
                                        bool check_rw_perm, bool read_only) {
  int ret = _stat64(path, statinfo);

  if (ret && (errno == ENOENT || errno == ENOTDIR)) {
    /* file does not exist */

    return (DB_NOT_FOUND);

  } else if (ret) {
    /* file exists, but stat call failed */

    os_file_handle_error_no_exit(path, "stat", false);

    return (DB_FAIL);

  } else if (_S_IFDIR & statinfo->st_mode) {
    stat_info->type = OS_FILE_TYPE_DIR;

  } else if (_S_IFREG & statinfo->st_mode) {
    DWORD access = GENERIC_READ;

    if (!read_only) {
      access |= GENERIC_WRITE;
    }

    stat_info->type = OS_FILE_TYPE_FILE;

    /* Check if we can open it in read-only mode. */

    if (check_rw_perm) {
      HANDLE fh;

      fh = CreateFile((LPCTSTR)path,  // File to open
                      access, FILE_SHARE_READ,
                      NULL,                   // Default security
                      OPEN_EXISTING,          // Existing file only
                      FILE_ATTRIBUTE_NORMAL,  // Normal file
                      NULL);                  // No attr. template

      if (fh == INVALID_HANDLE_VALUE) {
        stat_info->rw_perm = false;
      } else {
        stat_info->rw_perm = true;
        CloseHandle(fh);
      }
    }

    char volname[MAX_PATH];
    BOOL result = GetVolumePathName(path, volname, MAX_PATH);

    if (!result) {
      ib::error(ER_IB_MSG_806)
          << "os_file_get_status_win32: "
          << "Failed to get the volume path name for: " << path
          << "- OS error number " << GetLastError();

      return (DB_FAIL);
    }

    DWORD sectorsPerCluster;
    DWORD bytesPerSector;
    DWORD numberOfFreeClusters;
    DWORD totalNumberOfClusters;

    result =
        GetDiskFreeSpace((LPCSTR)volname, &sectorsPerCluster, &bytesPerSector,
                         &numberOfFreeClusters, &totalNumberOfClusters);

    if (!result) {
      ib::error(ER_IB_MSG_807) << "GetDiskFreeSpace(" << volname << ",...) "
                               << "failed "
                               << "- OS error number " << GetLastError();

      return (DB_FAIL);
    }

    stat_info->block_size = bytesPerSector * sectorsPerCluster;

    /* On Windows the block size is not used as the allocation
    unit for sparse files. The underlying infra-structure for
    sparse files is based on NTFS compression. The punch hole
    is done on a "compression unit". This compression unit
    is based on the cluster size. You cannot punch a hole if
    the cluster size >= 8K. For smaller sizes the table is
    as follows:

    Cluster Size	Compression Unit
    512 Bytes		 8 KB
      1 KB			16 KB
      2 KB			32 KB
      4 KB			64 KB

    Default NTFS cluster size is 4K, compression unit size of 64K.
    Therefore unless the user has created the file system with
    a smaller cluster size and used larger page sizes there is
    little benefit from compression out of the box. */

    stat_info->block_size = (stat_info->block_size <= 4096)
                                ? stat_info->block_size * 16
                                : UINT32_UNDEFINED;
  } else {
    stat_info->type = OS_FILE_TYPE_UNKNOWN;
  }

  return (DB_SUCCESS);
}

/** Truncates a file to a specified size in bytes.
Do nothing if the size to preserve is greater or equal to the current
size of the file.
@param[in]	pathname	file path
@param[in]	file		file to be truncated
@param[in]	size		size to preserve in bytes
@return true if success */
static bool os_file_truncate_win32(const char *pathname, pfs_os_file_t file,
                                   os_offset_t size) {
  LARGE_INTEGER length;

  length.QuadPart = size;

  BOOL success = SetFilePointerEx(file.m_file, length, NULL, FILE_BEGIN);

  if (!success) {
    os_file_handle_error_no_exit(pathname, "SetFilePointerEx", false);
  } else {
    success = SetEndOfFile(file.m_file);
    if (!success) {
      os_file_handle_error_no_exit(pathname, "SetEndOfFile", false);
    }
  }
  return (success);
}

/** Truncates a file at its current position.
@param[in]	file		Handle to be truncated
@return true if success */
bool os_file_set_eof(FILE *file) {
  HANDLE h = (HANDLE)_get_osfhandle(fileno(file));

  return (SetEndOfFile(h));
}

#ifdef UNIV_HOTBACKUP
/** Closes a file handle.
@param[in]	file		Handle to close
@return true if success */
bool os_file_close_no_error_handling(os_file_t file) {
  return (CloseHandle(file) ? true : false);
}
#endif /* UNIV_HOTBACKUP */

/** This function can be called if one wants to post a batch of reads and
prefers an i/o-handler thread to handle them all at once later. You must
call os_aio_simulated_wake_handler_threads later to ensure the threads
are not left sleeping! */
void os_aio_simulated_put_read_threads_to_sleep() {
  AIO::simulated_put_read_threads_to_sleep();
}

/** This function can be called if one wants to post a batch of reads and
prefers an i/o-handler thread to handle them all at once later. You must
call os_aio_simulated_wake_handler_threads later to ensure the threads
are not left sleeping! */
void AIO::simulated_put_read_threads_to_sleep() {
  /* The idea of putting background IO threads to sleep is only for
  Windows when using simulated AIO. Windows XP seems to schedule
  background threads too eagerly to allow for coalescing during
  readahead requests. */

  if (srv_use_native_aio) {
    /* We do not use simulated AIO: do nothing */

    return;
  }

  os_aio_recommend_sleep_for_read_threads = true;

  for (ulint i = 0; i < os_aio_n_segments; i++) {
    AIO *array;

    get_array_and_local_segment(&array, i);

    if (array == s_reads) {
      os_event_reset(os_aio_segment_wait_events[i]);
    }
  }
}

/** Depth first traversal of the directory starting from basedir
@param[in]	basedir		Start scanning from this directory
@param[in]      recursive       true if scan should be recursive
@param[in]	f		Callback for each entry found
@param[in,out]	args		Optional arguments for f */
void Dir_Walker::walk_win32(const Path &basedir, bool recursive, Function &&f) {
  using Stack = std::stack<Entry>;

  HRESULT res;
  size_t length;
  Stack directories;
  TCHAR directory[MAX_PATH];

  res = StringCchLength(basedir.c_str(), MAX_PATH, &length);

  /* Check if the name is too long. */
  if (!SUCCEEDED(res)) {
    ib::warn(ER_IB_MSG_808) << "StringCchLength() call failed!";
    return;

  } else if (length > (MAX_PATH - 3)) {
    ib::warn(ER_IB_MSG_809) << "Directory name too long: '" << basedir << "'";
    return;
  }

  StringCchCopy(directory, MAX_PATH, basedir.c_str());

  if (directory[_tcslen(directory) - 1] != TEXT('\\')) {
    StringCchCat(directory, MAX_PATH, TEXT("\\*"));
  } else {
    StringCchCat(directory, MAX_PATH, TEXT("*"));
  }

  directories.push(Entry(directory, 0));

  using Type = std::codecvt_utf8<wchar_t>;
  using Converter = std::wstring_convert<Type, wchar_t>;

  Converter converter;

  while (!directories.empty()) {
    Entry current = directories.top();

    directories.pop();

    HANDLE h;
    WIN32_FIND_DATA dirent;

    h = FindFirstFile(current.m_path.c_str(), &dirent);

    if (h == INVALID_HANDLE_VALUE) {
      ib::info(ER_IB_MSG_810) << "Directory read failed:"
                              << " '" << current.m_path << "' during scan";

      continue;
    }

    do {
      /* dirent.cFileName is a TCHAR. */
      if (_tcscmp(dirent.cFileName, _T(".")) == 0 ||
          _tcscmp(dirent.cFileName, _T("..")) == 0) {
        continue;
      }

      Path path(current.m_path);

      /* Shorten the path to remove the trailing '*'. */
      ut_ad(path.substr(path.size() - 2).compare("\\*") == 0);

      path.resize(path.size() - 1);
      path.append(dirent.cFileName);

      if ((dirent.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && recursive) {
        path.append("\\*");

        using value_type = Stack::value_type;

        value_type dir(path, current.m_depth + 1);

        directories.push(dir);

      } else {
        f(path, current.m_depth + 1);
      }

    } while (FindNextFile(h, &dirent) != 0);

    if (GetLastError() != ERROR_NO_MORE_FILES) {
      ib::error(ER_IB_MSG_811) << "Scanning '" << directory << "'"
                               << " - FindNextFile(): returned error";
    }

    FindClose(h);
  }
}
#endif /* !_WIN32*/


/** Does a synchronous write operation in Posix.
@param[in]	type		IO context
@param[in]	file		handle to an open file
@param[out]	buf		buffer from which to write
@param[in]	n		number of bytes to read, starting from offset
@param[in]	offset		file offset from the start where to read
@param[out]	err		DB_SUCCESS or error code
@return number of bytes written, -1 if error */
static MY_ATTRIBUTE((warn_unused_result)) ssize_t
    os_file_pwrite(IORequest &type, os_file_t file, const byte *buf, ulint n,
                   os_offset_t offset, dberr_t *err) {
#ifdef UNIV_HOTBACKUP
  static meb::Mutex meb_mutex;
#endif /* UNIV_HOTBACKUP */

  ut_ad(type.validate());

#ifdef UNIV_HOTBACKUP
  meb_mutex.lock();
#endif /* UNIV_HOTBACKUP */
  ++os_n_file_writes;
#ifdef UNIV_HOTBACKUP
  meb_mutex.unlock();
#endif /* UNIV_HOTBACKUP */

  (void)os_atomic_increment_ulint(&os_n_pending_writes, 1);
  MONITOR_ATOMIC_INC(MONITOR_OS_PENDING_WRITES);

  ssize_t n_bytes = os_file_io(type, file, (void *)buf, n, offset, err);

  (void)os_atomic_decrement_ulint(&os_n_pending_writes, 1);
  MONITOR_ATOMIC_DEC(MONITOR_OS_PENDING_WRITES);

  return (n_bytes);
}

/** Requests a synchronous write operation.
@param[in]	type		IO flags
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	file		handle to an open file
@param[out]	buf		buffer from which to write
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@return DB_SUCCESS if request was successful, false if fail */
static MY_ATTRIBUTE((warn_unused_result)) dberr_t
    os_file_write_page(IORequest &type, const char *name, os_file_t file,
                       const byte *buf, os_offset_t offset, ulint n) {
  dberr_t err;

  ut_ad(type.validate());
  ut_ad(n > 0);

  ssize_t n_bytes = os_file_pwrite(type, file, buf, n, offset, &err);

  if ((ulint)n_bytes != n && !os_has_said_disk_full) {
    ib::error(ER_IB_MSG_814) << "Write to file " << name << " failed at offset "
                             << offset << ", " << n
                             << " bytes should have been written,"
                                " only "
                             << n_bytes
                             << " were written."
                                " Operating system error number "
                             << errno
                             << "."
                                " Check that your OS and file system"
                                " support files of this size."
                                " Check also that the disk is not full"
                                " or a disk quota exceeded.";

    if (strerror(errno) != NULL) {
      ib::error(ER_IB_MSG_815)
          << "Error number " << errno << " means '" << strerror(errno) << "'";
    }

    ib::info(ER_IB_MSG_816) << OPERATING_SYSTEM_ERROR_MSG;

    os_has_said_disk_full = true;
  }

  return (err);
}

/** Does a synchronous read operation in Posix.
@param[in]	type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@param[out]	err		DB_SUCCESS or error code
@return number of bytes read, -1 if error */
static MY_ATTRIBUTE((warn_unused_result)) ssize_t
    os_file_pread(IORequest &type, os_file_t file, void *buf, ulint n,
                  os_offset_t offset, dberr_t *err) {
#ifdef UNIV_HOTBACKUP
  static meb::Mutex meb_mutex;

  meb_mutex.lock();
#endif /* UNIV_HOTBACKUP */
  ++os_n_file_reads;
#ifdef UNIV_HOTBACKUP
  meb_mutex.unlock();
#endif /* UNIV_HOTBACKUP */

  (void)os_atomic_increment_ulint(&os_n_pending_reads, 1);
  MONITOR_ATOMIC_INC(MONITOR_OS_PENDING_READS);

  ssize_t n_bytes = os_file_io(type, file, buf, n, offset, err);

  (void)os_atomic_decrement_ulint(&os_n_pending_reads, 1);
  MONITOR_ATOMIC_DEC(MONITOR_OS_PENDING_READS);

  return (n_bytes);
}

/** Requests a synchronous positioned read operation.
@return DB_SUCCESS if request was successful, false if fail
@param[in]	type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@param[out]	o		number of bytes actually read
@param[in]	exit_on_err	if true then exit on error
@return DB_SUCCESS or error code */
static MY_ATTRIBUTE((warn_unused_result)) dberr_t
    os_file_read_page(IORequest &type, os_file_t file, void *buf,
                      os_offset_t offset, ulint n, ulint *o, bool exit_on_err) {
  dberr_t err;

#ifdef UNIV_HOTBACKUP
  static meb::Mutex meb_mutex;

  meb_mutex.lock();
#endif /* UNIV_HOTBACKUP */
  os_bytes_read_since_printout += n;
#ifdef UNIV_HOTBACKUP
  meb_mutex.unlock();
#endif /* UNIV_HOTBACKUP */

  ut_ad(type.validate());
  ut_ad(n > 0);

  for (;;) {
    ssize_t n_bytes;

    n_bytes = os_file_pread(type, file, buf, n, offset, &err);

    if (o != NULL) {
      *o = n_bytes;
    }

    if (err != DB_SUCCESS && !exit_on_err) {
      return (err);

    } else if ((ulint)n_bytes == n) {
      /** The read will succeed but decompress can fail
      for various reasons. */

      if (type.is_compression_enabled() &&
          !Compression::is_compressed_page(static_cast<byte *>(buf))) {
        return (DB_SUCCESS);

      } else {
        return (err);
      }
    }

    ib::error(ER_IB_MSG_817)
        << "Tried to read " << n << " bytes at offset " << offset
        << ", but was only able to read " << n_bytes;

    if (exit_on_err) {
      if (!os_file_handle_error(NULL, "read")) {
        /* Hard error */
        break;
      }

    } else if (!os_file_handle_error_no_exit(NULL, "read", false)) {
      /* Hard error */
      break;
    }

    if (n_bytes > 0 && (ulint)n_bytes < n) {
      n -= (ulint)n_bytes;
      offset += (ulint)n_bytes;
      buf = reinterpret_cast<uchar *>(buf) + (ulint)n_bytes;
    }
  }

  ib::fatal(ER_IB_MSG_818) << "Cannot read from file. OS error number " << errno
                           << ".";

  return (err);
}









/**  Write the specified number of zeros to a file from specific offset.
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	file		handle to a file
@param[in]	offset		file offset
@param[in]	size		file size
@param[in]	read_only	Enable read-only checks if true
@param[in]	flush		Flush file content to disk
@return true if success */
bool os_file_set_size(const char *name, pfs_os_file_t file, os_offset_t offset,
                      os_offset_t size, bool read_only, bool flush) {
  /* Write up to FSP_EXTENT_SIZE bytes at a time. */
  ulint buf_size = 0;

  if (size <= UNIV_PAGE_SIZE) {
    buf_size = 1;
  } else {
    buf_size = std::min(static_cast<ulint>(64),
                      static_cast<ulint>(size / UNIV_PAGE_SIZE));
  }

  ut_ad(buf_size != 0);

  buf_size *= UNIV_PAGE_SIZE;

  /* Align the buffer for possible raw i/o */
  byte *buf2;

  buf2 = static_cast<byte *>(ut_malloc_nokey(buf_size + UNIV_PAGE_SIZE));

  byte *buf = static_cast<byte *>(ut_align(buf2, UNIV_PAGE_SIZE));

  /* Write buffer full of zeros */
  memset(buf, 0, buf_size);

  if (size >= (os_offset_t)100 << 20) {
    ib::info(ER_IB_MSG_826) << "Progress in MB:";
  }

  os_offset_t current_size = offset;

  while (current_size < size) {
    ulint n_bytes;

    if (size - current_size < (os_offset_t)buf_size) {
      n_bytes = (ulint)(size - current_size);
    } else {
      n_bytes = buf_size;
    }

    dberr_t err;
    IORequest request(IORequest::WRITE);

#ifdef UNIV_HOTBACKUP

    err = os_file_write(request, name, file, buf, current_size, n_bytes);
#else
    /* Using AIO_mode::SYNC mode on POSIX systems will result in
    fall back to os_file_write/read. On Windows it will use
    special mechanism to wait before it returns back. */

    err = os_aio(request, AIO_mode::SYNC, name, file, buf, current_size,
                 n_bytes, read_only, NULL, NULL);
#endif /* UNIV_HOTBACKUP */

    if (err != DB_SUCCESS) {
      ut_free(buf2);
      return (false);
    }

    /* Print about progress for each 100 MB written */
    if ((current_size + n_bytes) / (100 << 20) != current_size / (100 << 20)) {
      fprintf(stderr, " %lu00",
              (ulong)((current_size + n_bytes) / (100 << 20)));
    }

    /* Flush after each os_fsync_threhold bytes */
    if (flush && os_fsync_threshold != 0) {
      if ((current_size + n_bytes) / os_fsync_threshold !=
          current_size / os_fsync_threshold) {
        DBUG_EXECUTE_IF("flush_after_reaching_threshold",
                        std::cerr << os_fsync_threshold
                                  << " bytes being flushed at once"
                                  << std::endl;);

        bool ret = os_file_flush(file);

        if (!ret) {
          ut_free(buf2);
          return (false);
        }
      }
    }

    current_size += n_bytes;
  }

  if (size >= (os_offset_t)100 << 20) {
    fprintf(stderr, "\n");
  }

  ut_free(buf2);

  if (flush) {
    return (os_file_flush(file));
  }

  return (true);
}


/** NOTE! Use the corresponding macro os_file_read(), not directly this
function!
Requests a synchronous positioned read operation.
@return DB_SUCCESS if request was successful, DB_IO_ERROR on failure
@param[in]	type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@return DB_SUCCESS or error code */
dberr_t os_file_read_func(IORequest &type, os_file_t file, void *buf,
                          os_offset_t offset, ulint n) {
  ut_ad(type.is_read());

  return (os_file_read_page(type, file, buf, offset, n, NULL, true));
}

/** NOTE! Use the corresponding macro os_file_read_first_page(), not
directly this function!
Requests a synchronous positioned read operation of page 0 of IBD file
@return DB_SUCCESS if request was successful, DB_IO_ERROR on failure
@param[in]	type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	n		number of bytes to read, starting from offset
@return DB_SUCCESS or error code */
dberr_t os_file_read_first_page_func(IORequest &type, os_file_t file, void *buf,
                                     ulint n) {
  ut_ad(type.is_read());

  dberr_t err =
      os_file_read_page(type, file, buf, 0, UNIV_ZIP_SIZE_MIN, NULL, true);

  if (err == DB_SUCCESS) {
    uint32_t flags = fsp_header_get_flags(static_cast<byte *>(buf));
    const page_size_t page_size(flags);
    ut_ad(page_size.physical() <= n);
    err =
        os_file_read_page(type, file, buf, 0, page_size.physical(), NULL, true);
  }
  return (err);
}

/** copy data from one file to another file using read, write.
@param[in]	src_file	file handle to copy from
@param[in]	src_offset	offset to copy from
@param[in]	dest_file	file handle to copy to
@param[in]	dest_offset	offset to copy to
@param[in]	size		number of bytes to copy
@return DB_SUCCESS if successful */
static dberr_t os_file_copy_read_write(os_file_t src_file,
                                       os_offset_t src_offset,
                                       os_file_t dest_file,
                                       os_offset_t dest_offset, uint size) {
  dberr_t err;
  uint request_size;
  const uint BUF_SIZE = 4 * UNIV_SECTOR_SIZE;

  char buf[BUF_SIZE + UNIV_SECTOR_SIZE];
  char *buf_ptr;

  buf_ptr = static_cast<char *>(ut_align(buf, UNIV_SECTOR_SIZE));

  IORequest read_request(IORequest::READ);
  read_request.disable_compression();
  read_request.clear_encrypted();

  IORequest write_request(IORequest::WRITE);
  write_request.disable_compression();
  write_request.clear_encrypted();

  while (size > 0) {
    if (size > BUF_SIZE) {
      request_size = BUF_SIZE;
    } else {
      request_size = size;
    }

    err = os_file_read_func(read_request, src_file, buf_ptr, src_offset,
                            request_size);

    if (err != DB_SUCCESS) {
      return (err);
    }
    src_offset += request_size;

    err = os_file_write_func(write_request, "file copy", dest_file, buf_ptr,
                             dest_offset, request_size);

    if (err != DB_SUCCESS) {
      return (err);
    }
    dest_offset += request_size;
    size -= request_size;
  }

  return (DB_SUCCESS);
}

/** copy data from one file to another file.
@param[in]	src_file	file handle to copy from
@param[in]	src_offset	offset to copy from
@param[in]	dest_file	file handle to copy to
@param[in]	dest_offset	offset to copy to
@param[in]	size		number of bytes to copy
@return DB_SUCCESS if successful */
#ifdef __linux__
dberr_t os_file_copy_func(os_file_t src_file, os_offset_t src_offset,
                          os_file_t dest_file, os_offset_t dest_offset,
                          uint size) {
  dberr_t err;
  static bool use_sendfile = true;

  uint actual_size;
  int ret_size;

  int src_fd;
  int dest_fd;

  if (!os_file_seek(nullptr, src_file, src_offset)) {
    return (DB_IO_ERROR);
  }

  if (!os_file_seek(nullptr, dest_file, dest_offset)) {
    return (DB_IO_ERROR);
  }

  src_fd = OS_FD_FROM_FILE(src_file);
  dest_fd = OS_FD_FROM_FILE(dest_file);

  while (use_sendfile && size > 0) {
    ret_size = sendfile(dest_fd, src_fd, nullptr, size);

    if (ret_size == -1) {
      /* Fall through read/write path. */
      ib::info(ER_IB_MSG_827) << "sendfile failed to copy data"
                                 " : trying read/write ";

      use_sendfile = false;
      break;
    }

    actual_size = static_cast<uint>(ret_size);

    ut_ad(size >= actual_size);
    size -= actual_size;
  }

  if (size == 0) {
    return (DB_SUCCESS);
  }

  err = os_file_copy_read_write(src_file, src_offset, dest_file, dest_offset,
                                size);

  return (err);
}
#else
dberr_t os_file_copy_func(os_file_t src_file, os_offset_t src_offset,
                          os_file_t dest_file, os_offset_t dest_offset,
                          uint size) {
  dberr_t err;

  err = os_file_copy_read_write(src_file, src_offset, dest_file, dest_offset,
                                size);
  return (err);
}
#endif

/** NOTE! Use the corresponding macro os_file_read_no_error_handling(),
not directly this function!
Requests a synchronous positioned read operation.
@return DB_SUCCESS if request was successful, DB_IO_ERROR on failure
@param[in]	type		IO flags
@param[in]	file		handle to an open file
@param[out]	buf		buffer where to read
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@param[out]	o		number of bytes actually read
@return DB_SUCCESS or error code */
dberr_t os_file_read_no_error_handling_func(IORequest &type, os_file_t file,
                                            void *buf, os_offset_t offset,
                                            ulint n, ulint *o) {
  ut_ad(type.is_read());

  return (os_file_read_page(type, file, buf, offset, n, o, false));
}

/** NOTE! Use the corresponding macro os_file_write(), not directly
Requests a synchronous write operation.
@param[in]	type		IO flags
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	file		handle to an open file
@param[out]	buf		buffer from which to write
@param[in]	offset		file offset from the start where to read
@param[in]	n		number of bytes to read, starting from offset
@return DB_SUCCESS if request was successful, false if fail */
dberr_t os_file_write_func(IORequest &type, const char *name, os_file_t file,
                           const void *buf, os_offset_t offset, ulint n) {
  ut_ad(type.validate());
  ut_ad(type.is_write());

  /* We never compress the first page.
  Note: This assumes we always do block IO. */
  if (offset == 0) {
    type.clear_compressed();
  }

  const byte *ptr = reinterpret_cast<const byte *>(buf);

  return (os_file_write_page(type, name, file, ptr, offset, n));
}






/** Waits for an AIO operation to complete. This function is used to wait
for completed requests. The aio array of pending requests is divided
into segments. The thread specifies which segment or slot it wants to wait
for. NOTE: this function will also take care of freeing the aio slot,
therefore no other thread is allowed to do the freeing!
@param[in]	segment		The number of the segment in the aio arrays to
                                wait for; segment 0 is the ibuf I/O thread,
                                segment 1 the log I/O thread, then follow the
                                non-ibuf read threads, and as the last are the
                                non-ibuf write threads; if this is
                                ULINT_UNDEFINED, then it means that sync AIO
                                is used, and this parameter is ignored
@param[out]	m1		the messages passed with the AIO request; note
                                that also in the case where the AIO operation
                                failed, these output parameters are valid and
                                can be used to restart the operation,
                                for example
@param[out]	m2		callback message
@param[out]	request		OS_FILE_WRITE or ..._READ
@return DB_SUCCESS or error code */
dberr_t os_aio_handler(ulint segment, fil_node_t **m1, void **m2,
                       IORequest *request) {
  dberr_t err;

  if (srv_use_native_aio) {
    srv_set_io_thread_op_info(segment, "native aio handle");

#ifdef WIN_ASYNC_IO

    err = os_aio_windows_handler(segment, 0, m1, m2, request);

#elif defined(LINUX_NATIVE_AIO)

    err = os_aio_linux_handler(segment, m1, m2, request);

#else
    ut_error;

    err = DB_ERROR; /* Eliminate compiler warning */

#endif /* WIN_ASYNC_IO */

  } else {
    srv_set_io_thread_op_info(segment, "simulated aio handle");

    err = os_aio_simulated_handler(segment, m1, m2, request);
  }

  return (err);
}





/** Initialise the array */
dberr_t AIO::init() {
  ut_a(!m_slots.empty());

#ifdef _WIN32
  ut_a(m_handles == NULL);

  m_handles = UT_NEW_NOKEY(Handles(m_slots.size()));
#endif /* _WIN32 */

  if (srv_use_native_aio) {
#ifdef LINUX_NATIVE_AIO
    dberr_t err = init_linux_native_aio();

    if (err != DB_SUCCESS) {
      return (err);
    }

#endif /* LINUX_NATIVE_AIO */
  }

  return (init_slots());
}
#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)

/** Max disk sector size */
static const ulint MAX_SECTOR_SIZE = 4096;

/**
Try and get the FusionIO sector size. */
void os_fusionio_get_sector_size() {
  if (srv_unix_file_flush_method == SRV_UNIX_O_DIRECT ||
      srv_unix_file_flush_method == SRV_UNIX_O_DIRECT_NO_FSYNC) {
    ulint sector_size = UNIV_SECTOR_SIZE;
    char *path = srv_data_home;
    os_file_t check_file;
    byte *ptr;
    byte *block_ptr;
    char current_dir[3];
    char *dir_end;
    ulint dir_len;
    ulint check_path_len;
    char *check_file_name;
    ssize_t ret;

    /* If the srv_data_home is empty, set the path to
    current dir. */
    if (*path == 0) {
      current_dir[0] = FN_CURLIB;
      current_dir[1] = FN_LIBCHAR;
      current_dir[2] = 0;
      path = current_dir;
    }

    /* Get the path of data file */
    dir_end = strrchr(path, OS_PATH_SEPARATOR);
    dir_len = dir_end ? dir_end - path : strlen(path);

    /* allocate a new path and move the directory path to it. */
    check_path_len = dir_len + sizeof "/check_sector_size";
    check_file_name = static_cast<char *>(ut_zalloc_nokey(check_path_len));
    memcpy(check_file_name, path, dir_len);

    /* Construct a check file name. */
    strcat(check_file_name + dir_len, "/check_sector_size");

    /* Create a tmp file for checking sector size. */
    check_file = ::open(check_file_name,
                        O_CREAT | O_TRUNC | O_WRONLY | O_DIRECT, S_IRWXU);

    if (check_file == -1) {
      ib::error(ER_IB_MSG_830)
          << "Failed to create check sector file, errno:" << errno
          << " Please confirm O_DIRECT is"
          << " supported and remove the file " << check_file_name
          << " if it exists.";
      ut_free(check_file_name);
      errno = 0;
      return;
    }

    /* Try to write the file with different sector size
    alignment. */
    ptr = static_cast<byte *>(ut_zalloc_nokey(2 * MAX_SECTOR_SIZE));

    while (sector_size <= MAX_SECTOR_SIZE) {
      block_ptr = static_cast<byte *>(ut_align(ptr, sector_size));
      ret = pwrite(check_file, block_ptr, sector_size, 0);
      if (ret > 0 && (ulint)ret == sector_size) {
        break;
      }
      sector_size *= 2;
    }

    /* The sector size should <= MAX_SECTOR_SIZE. */
    ut_ad(sector_size <= MAX_SECTOR_SIZE);

    close(check_file);
    unlink(check_file_name);

    ut_free(check_file_name);
    ut_free(ptr);
    errno = 0;

    os_io_ptr_align = sector_size;
  }
}
#endif /* !NO_FALLOCATE && UNIV_LINUX */





/** Initializes the asynchronous io system. Creates one array each for ibuf
and log i/o. Also creates one array each for read and write where each
array is divided logically into n_readers and n_writers
respectively. The caller must create an i/o handler thread for each
segment in these arrays. This function also creates the sync array.
No i/o handler thread needs to be created for that
@param[in]	n_readers	number of reader threads
@param[in]	n_writers	number of writer threads
@param[in]	n_slots_sync	number of slots in the sync aio array */
bool os_aio_init(ulint n_readers, ulint n_writers, ulint n_slots_sync) {
  /* Maximum number of pending aio operations allowed per segment */
  ulint limit = 8 * OS_AIO_N_PENDING_IOS_PER_THREAD;

#ifdef _WIN32
  if (srv_use_native_aio) {
    limit = SRV_N_PENDING_IOS_PER_THREAD;
  }
#endif /* _WIN32 */

    /* Get sector size for DIRECT_IO. In this case, we need to
    know the sector size for aligning the write buffer. */
#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)
  os_fusionio_get_sector_size();
#endif /* !NO_FALLOCATE && UNIV_LINUX */

  return (AIO::start(limit, n_readers, n_writers, n_slots_sync));
}

/** Frees the asynchronous io system. */
void os_aio_free() {
  AIO::shutdown();

  for (ulint i = 0; i < os_aio_n_segments; i++) {
    os_event_destroy(os_aio_segment_wait_events[i]);
  }

  ut_free(os_aio_segment_wait_events);
  os_aio_segment_wait_events = 0;
  os_aio_n_segments = 0;

  for (Blocks::iterator it = block_cache->begin(); it != block_cache->end();
       ++it) {
    ut_a(it->m_in_use == 0);
    ut_free(it->m_ptr);
  }

  UT_DELETE(block_cache);

  block_cache = NULL;
}

/** Wakes up all async i/o threads so that they know to exit themselves in
shutdown. */
void os_aio_wake_all_threads_at_shutdown() {
#ifdef WIN_ASYNC_IO

  AIO::wake_at_shutdown();

#elif defined(LINUX_NATIVE_AIO)

  /* When using native AIO interface the io helper threads
  wait on io_getevents with a timeout value of 500ms. At
  each wake up these threads check the server status.
  No need to do anything to wake them up. */

  if (srv_use_native_aio) {
    return;
  }

#endif /* !WIN_ASYNC_AIO */

  /* Fall through to simulated AIO handler wakeup if we are
  not using native AIO. */

  /* This loop wakes up all simulated ai/o threads */

  for (ulint i = 0; i < os_aio_n_segments; ++i) {
    os_event_set(os_aio_segment_wait_events[i]);
  }
}

/** Waits until there are no pending writes in AIO::s_writes. There can
be other, synchronous, pending writes. */
void os_aio_wait_until_no_pending_writes() {
  AIO::wait_until_no_pending_writes();
}

/** Requests for a slot in the aio array. If no slot is available, waits until
not_full-event becomes signaled.

@param[in,out]	type		IO context
@param[in,out]	m1		message to be passed along with the AIO
                                operation
@param[in,out]	m2		message to be passed along with the AIO
                                operation
@param[in]	file		file handle
@param[in]	name		name of the file or path as a NUL-terminated
                                string
@param[in,out]	buf		buffer where to read or from which to write
@param[in]	offset		file offset, where to read from or start writing
@param[in]	len		length of the block to read or write
@return pointer to slot */
Slot *AIO::reserve_slot(IORequest &type, fil_node_t *m1, void *m2,
                        pfs_os_file_t file, const char *name, void *buf,
                        os_offset_t offset, ulint len) {
#ifdef WIN_ASYNC_IO
  ut_a((len & 0xFFFFFFFFUL) == len);
#endif /* WIN_ASYNC_IO */

  /* No need of a mutex. Only reading constant fields */
  ulint slots_per_seg;

  ut_ad(type.validate());

  slots_per_seg = slots_per_segment();

  /* We attempt to keep adjacent blocks in the same local
  segment. This can help in merging IO requests when we are
  doing simulated AIO */
  ulint local_seg;

  local_seg = (offset >> (UNIV_PAGE_SIZE_SHIFT + 6)) % m_n_segments;

  for (;;) {
    acquire();

    if (m_n_reserved != m_slots.size()) {
      break;
    }

    release();

    if (!srv_use_native_aio) {
      /* If the handler threads are suspended,
      wake them so that we get more slots */

      os_aio_simulated_wake_handler_threads();
    }

    os_event_wait(m_not_full);
  }

  ulint counter = 0;
  Slot *slot = NULL;

  /* We start our search for an available slot from our preferred
  local segment and do a full scan of the array. We are
  guaranteed to find a slot in full scan. */
  for (ulint i = local_seg * slots_per_seg; counter < m_slots.size();
       ++i, ++counter) {
    i %= m_slots.size();

    slot = at(i);

    if (slot->is_reserved == false) {
      break;
    }
  }

  /* We MUST always be able to get hold of a reserved slot. */
  ut_a(counter < m_slots.size());

  ut_a(slot->is_reserved == false);

  ++m_n_reserved;

  if (m_n_reserved == 1) {
    os_event_reset(m_is_empty);
  }

  if (m_n_reserved == m_slots.size()) {
    os_event_reset(m_not_full);
  }

  slot->is_reserved = true;
  slot->reservation_time = ut_time();
  slot->m1 = m1;
  slot->m2 = m2;
  slot->file = file;
  slot->name = name;
#ifdef _WIN32
  slot->len = static_cast<DWORD>(len);
#else
  slot->len = static_cast<ulint>(len);
#endif /* _WIN32 */
  slot->type = type;
  slot->buf = static_cast<byte *>(buf);
  slot->ptr = slot->buf;
  slot->offset = offset;
  slot->err = DB_SUCCESS;
  slot->original_len = static_cast<uint32>(len);
  slot->io_already_done = false;
  slot->buf_block = NULL;
  slot->encrypt_log_buf = NULL;

  if (srv_use_native_aio && offset > 0 && type.is_write() &&
      type.is_compressed()) {
    ulint compressed_len = len;

    ut_ad(!type.is_log());

    release();

    void *src_buf = slot->buf;
    slot->buf_block = os_file_compress_page(type, src_buf, &compressed_len);

    slot->buf = static_cast<byte *>(src_buf);
    slot->ptr = slot->buf;
#ifdef _WIN32
    slot->len = static_cast<DWORD>(compressed_len);
#else
    slot->len = static_cast<ulint>(compressed_len);
#endif /* _WIN32 */
    slot->skip_punch_hole = !type.punch_hole();

    acquire();
  }

  /* We do encryption after compression, since if we do encryption
  before compression, the encrypted data will cause compression fail
  or low compression rate. */
  if (srv_use_native_aio && offset > 0 && type.is_write() &&
      type.is_encrypted()) {
    ulint encrypted_len = slot->len;
    Block *encrypted_block;
    byte *encrypt_log_buf;

    release();

    void *src_buf = slot->buf;
    if (!type.is_log()) {
      encrypted_block = os_file_encrypt_page(type, src_buf, &encrypted_len);

      if (slot->buf_block != NULL) {
        os_free_block(slot->buf_block);
      }

      slot->buf_block = encrypted_block;
    } else {
      /* Skip encrypt log file header */
      if (offset >= LOG_FILE_HDR_SIZE) {
        encrypted_block =
            os_file_encrypt_log(type, src_buf, encrypt_log_buf, &encrypted_len);

        if (slot->buf_block != NULL) {
          os_free_block(slot->buf_block);
        }

        slot->buf_block = encrypted_block;

        if (slot->encrypt_log_buf != NULL) {
          ut_free(slot->encrypt_log_buf);
        }

        slot->encrypt_log_buf = encrypt_log_buf;
      }
    }

    slot->buf = static_cast<byte *>(src_buf);

    slot->ptr = slot->buf;

#ifdef _WIN32
    slot->len = static_cast<DWORD>(encrypted_len);
#else
    slot->len = static_cast<ulint>(encrypted_len);
#endif /* _WIN32 */

    acquire();
  }

#ifdef WIN_ASYNC_IO
  {
    OVERLAPPED *control;

    control = &slot->control;
    control->Offset = (DWORD)offset & 0xFFFFFFFF;
    control->OffsetHigh = (DWORD)(offset >> 32);

    ResetEvent(slot->handle);
  }
#elif defined(LINUX_NATIVE_AIO)

  /* If we are not using native AIO skip this part. */
  if (srv_use_native_aio) {
    off_t aio_offset;

    /* Check if we are dealing with 64 bit arch.
    If not then make sure that offset fits in 32 bits. */
    aio_offset = (off_t)offset;

    ut_a(sizeof(aio_offset) >= sizeof(offset) ||
         ((os_offset_t)aio_offset) == offset);

    struct iocb *iocb = &slot->control;

    if (type.is_read()) {
      io_prep_pread(iocb, file.m_file, slot->ptr, slot->len, aio_offset);
    } else {
      ut_ad(type.is_write());
      io_prep_pwrite(iocb, file.m_file, slot->ptr, slot->len, aio_offset);
    }

    iocb->data = slot;

    slot->n_bytes = 0;
    slot->ret = 0;
  }
#endif /* LINUX_NATIVE_AIO */

  release();

  return (slot);
}

/** Wakes up a simulated aio i/o-handler thread if it has something to do.
@param[in]	global_segment	The number of the segment in the AIO arrays */
void AIO::wake_simulated_handler_thread(ulint global_segment) {
  ut_ad(!srv_use_native_aio);

  AIO *array;
  ulint segment = get_array_and_local_segment(&array, global_segment);

  array->wake_simulated_handler_thread(global_segment, segment);
}

/** Wakes up a simulated AIO I/O-handler thread if it has something to do
for a local segment in the AIO array.
@param[in]	global_segment	The number of the segment in the AIO arrays
@param[in]	segment		The local segment in the AIO array */
void AIO::wake_simulated_handler_thread(ulint global_segment, ulint segment) {
  ut_ad(!srv_use_native_aio);

  ulint n = slots_per_segment();
  ulint offset = segment * n;

  /* Look through n slots after the segment * n'th slot */

  acquire();

  const Slot *slot = at(offset);

  for (ulint i = 0; i < n; ++i, ++slot) {
    if (slot->is_reserved) {
      /* Found an i/o request */

      release();

      os_event_t event;

      event = os_aio_segment_wait_events[global_segment];

      os_event_set(event);

      return;
    }
  }

  release();
}

/** Wakes up simulated aio i/o-handler threads if they have something to do. */
void os_aio_simulated_wake_handler_threads() {
  if (srv_use_native_aio) {
    /* We do not use simulated aio: do nothing */

    return;
  }

  os_aio_recommend_sleep_for_read_threads = false;

  for (ulint i = 0; i < os_aio_n_segments; i++) {
    AIO::wake_simulated_handler_thread(i);
  }
}

/** Select the IO slot array
@param[in,out]	type		Type of IO, READ or WRITE
@param[in]	read_only	true if running in read-only mode
@param[in]	aio_mode	IO mode
@return slot array or NULL if invalid mode specified */
AIO *AIO::select_slot_array(IORequest &type, bool read_only,
                            AIO_mode aio_mode) {
  AIO *array;

  ut_ad(type.validate());

  switch (aio_mode) {
    case AIO_mode::NORMAL:

      array = type.is_read() ? AIO::s_reads : AIO::s_writes;
      break;

    case AIO_mode::IBUF:
      ut_ad(type.is_read());

      /* Reduce probability of deadlock bugs in connection with ibuf:
      do not let the ibuf i/o handler sleep */

      type.clear_do_not_wake();

      array = read_only ? AIO::s_reads : AIO::s_ibuf;
      break;

    case AIO_mode::LOG:

      array = read_only ? AIO::s_reads : AIO::s_log;
      break;

    case AIO_mode::SYNC:

      array = AIO::s_sync;
#if defined(LINUX_NATIVE_AIO)
      /* In Linux native AIO we don't use sync IO array. */
      ut_a(!srv_use_native_aio);
#endif /* LINUX_NATIVE_AIO */
      break;

    default:
      ut_error;
  }

  return (array);
}

#ifdef WIN_ASYNC_IO
/** This function is only used in Windows asynchronous i/o.
Waits for an aio operation to complete. This function is used to wait the
for completed requests. The aio array of pending requests is divided
into segments. The thread specifies which segment or slot it wants to wait
for. NOTE: this function will also take care of freeing the aio slot,
therefore no other thread is allowed to do the freeing!
@param[in]	segment		The number of the segment in the aio arrays to
                                wait for; segment 0 is the ibuf I/O thread,
                                segment 1 the log I/O thread, then follow the
                                non-ibuf read threads, and as the last are the
                                non-ibuf write threads; if this is
                                ULINT_UNDEFINED, then it means that sync AIO
                                is used, and this parameter is ignored
@param[in]	pos		this parameter is used only in sync AIO:
                                wait for the aio slot at this position
@param[out]	m1		the messages passed with the AIO request; note
                                that also in the case where the AIO operation
                                failed, these output parameters are valid and
                                can be used to restart the operation,
                                for example
@param[out]	m2		callback message
@param[out]	type		OS_FILE_WRITE or ..._READ
@return DB_SUCCESS or error code */
static dberr_t os_aio_windows_handler(ulint segment, ulint pos, fil_node_t **m1,
                                      void **m2, IORequest *type) {
  Slot *slot;
  dberr_t err;
  AIO *array;
  ulint orig_seg = segment;

  if (segment == ULINT_UNDEFINED) {
    segment = 0;
    array = AIO::sync_array();
  } else {
    segment = AIO::get_array_and_local_segment(&array, segment);
  }

    /* NOTE! We only access constant fields in os_aio_array. Therefore
    we do not have to acquire the protecting mutex yet */

#ifndef UNIV_HOTBACKUP
  ut_ad(os_aio_validate_skip());
#endif /* !UNIV_HOTBACKUP */

  if (array == AIO::sync_array()) {
    WaitForSingleObject(array->at(pos)->handle, INFINITE);

  } else {
    if (orig_seg != ULINT_UNDEFINED) {
      srv_set_io_thread_op_info(orig_seg, "wait Windows aio");
    }

    pos = WaitForMultipleObjects((DWORD)array->slots_per_segment(),
                                 array->handles(segment), FALSE, INFINITE);
  }

  array->acquire();

  if (
#ifndef UNIV_HOTBACKUP
      srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS
#else  /* !UNIV_HOTBACKUP */
      true
#endif /* !UNIV_HOTBACKUP */
      && array->is_empty() && !buf_page_cleaner_is_active) {

    *m1 = NULL;
    *m2 = NULL;

    array->release();

    return (DB_SUCCESS);
  }

  ulint n = array->slots_per_segment();

  ut_a(pos >= WAIT_OBJECT_0 && pos <= WAIT_OBJECT_0 + n);

  slot = array->at(pos + segment * n);

  ut_a(slot->is_reserved);

  if (orig_seg != ULINT_UNDEFINED) {
    srv_set_io_thread_op_info(orig_seg, "get windows aio return value");
  }

  BOOL ret;
  ret = GetOverlappedResult(slot->file.m_file, &slot->control, &slot->n_bytes,
                            TRUE);

  *m1 = slot->m1;
  *m2 = slot->m2;

  *type = slot->type;

  BOOL retry = FALSE;

  if (ret && slot->n_bytes == slot->len) {
    err = DB_SUCCESS;

  } else if (os_file_handle_error(slot->name, "Windows aio")) {
    retry = true;

  } else {
    err = DB_IO_ERROR;
  }

  array->release();

  if (retry) {
  /* Retry failed read/write operation synchronously.
  No need to hold array->m_mutex. */

#ifdef UNIV_PFS_IO
    /* This read/write does not go through os_file_read
    and os_file_write APIs, need to register with
    performance schema explicitly here. */
    struct PSI_file_locker *locker = NULL;
    PSI_file_locker_state state;
    register_pfs_file_io_begin(
        &state, locker, slot->file, slot->len,
        slot->type.is_write() ? PSI_FILE_WRITE : PSI_FILE_READ, __FILE__,
        __LINE__);
#endif /* UNIV_PFS_IO */

    ut_a((slot->len & 0xFFFFFFFFUL) == slot->len);

    ssize_t n_bytes = SyncFileIO::execute(slot);

#ifdef UNIV_PFS_IO
    register_pfs_file_io_end(locker, slot->len);
#endif /* UNIV_PFS_IO */

    if (n_bytes < 0 && GetLastError() == ERROR_IO_PENDING) {
      /* AIO was queued successfully!
      We want a synchronous I/O operation on a
      file where we also use async I/O: in Windows
      we must use the same wait mechanism as for
      async I/O */

      BOOL ret;
      ret = GetOverlappedResult(slot->file.m_file, &slot->control,
                                &slot->n_bytes, TRUE);

      n_bytes = ret ? slot->n_bytes : -1;
    }

    err = (n_bytes == slot->len) ? DB_SUCCESS : DB_IO_ERROR;
  }

  if (err == DB_SUCCESS) {
    err = AIOHandler::post_io_processing(slot);
  }

  array->release_with_mutex(slot);

  return (err);
}
#endif /* WIN_ASYNC_IO */

/**
NOTE! Use the corresponding macro os_aio(), not directly this function!
Requests an asynchronous i/o operation.
@param[in]	type		IO request context
@param[in]	aio_mode	IO mode
@param[in]	name		Name of the file or path as NUL terminated
                                string
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@param[in]	read_only	if true read only mode checks are enforced
@param[in,out]	m1		Message for the AIO handler, (can be used to
                                identify a completed AIO operation); ignored
                                if mode is AIO_mode::SYNC
@param[in,out]	m2		message for the AIO handler (can be used to
                                identify a completed AIO operation); ignored
                                if mode is AIO_mode::SYNC
@return DB_SUCCESS or error code */
dberr_t os_aio_func(IORequest &type, AIO_mode aio_mode, const char *name,
                    pfs_os_file_t file, void *buf, os_offset_t offset, ulint n,
                    bool read_only, fil_node_t *m1, void *m2) {
#ifdef WIN_ASYNC_IO
  BOOL ret = TRUE;
#endif /* WIN_ASYNC_IO */

  ut_ad(n > 0);
  ut_ad((n % OS_FILE_LOG_BLOCK_SIZE) == 0);
  ut_ad((offset % OS_FILE_LOG_BLOCK_SIZE) == 0);
#ifndef UNIV_HOTBACKUP
  ut_ad(os_aio_validate_skip());
#endif /* !UNIV_HOTBACKUP */

#ifdef WIN_ASYNC_IO
  ut_ad((n & 0xFFFFFFFFUL) == n);
#endif /* WIN_ASYNC_IO */

  if (aio_mode == AIO_mode::SYNC
#ifdef WIN_ASYNC_IO
      && !srv_use_native_aio
#endif /* WIN_ASYNC_IO */
  ) {
    /* This is actually an ordinary synchronous read or write:
    no need to use an i/o-handler thread. NOTE that if we use
    Windows async i/o, Windows does not allow us to use
    ordinary synchronous os_file_read etc. on the same file,
    therefore we have built a special mechanism for synchronous
    wait in the Windows case.
    Also note that the Performance Schema instrumentation has
    been performed by current os_aio_func()'s wrapper function
    pfs_os_aio_func(). So we would no longer need to call
    Performance Schema instrumented os_file_read() and
    os_file_write(). Instead, we should use os_file_read_func()
    and os_file_write_func() */

    if (type.is_read()) {
      return (os_file_read_func(type, file.m_file, buf, offset, n));
    }

    ut_ad(type.is_write());
    return (os_file_write_func(type, name, file.m_file, buf, offset, n));
  }

try_again:

  AIO *array;

  array = AIO::select_slot_array(type, read_only, aio_mode);

  Slot *slot;

  slot = array->reserve_slot(type, m1, m2, file, name, buf, offset, n);

  if (type.is_read()) {
    if (srv_use_native_aio) {
      ++os_n_file_reads;

      os_bytes_read_since_printout += n;
#ifdef WIN_ASYNC_IO
      ret = ReadFile(file.m_file, slot->ptr, slot->len, &slot->n_bytes,
                     &slot->control);
#elif defined(LINUX_NATIVE_AIO)
      if (!array->linux_dispatch(slot)) {
        goto err_exit;
      }
#endif /* WIN_ASYNC_IO */
    } else if (type.is_wake()) {
      AIO::wake_simulated_handler_thread(
          AIO::get_segment_no_from_slot(array, slot));
    }
  } else if (type.is_write()) {
    if (srv_use_native_aio) {
      ++os_n_file_writes;

#ifdef WIN_ASYNC_IO
      ret = WriteFile(file.m_file, slot->ptr, slot->len, &slot->n_bytes,
                      &slot->control);
#elif defined(LINUX_NATIVE_AIO)
      if (!array->linux_dispatch(slot)) {
        goto err_exit;
      }
#endif /* WIN_ASYNC_IO */

    } else if (type.is_wake()) {
      AIO::wake_simulated_handler_thread(
          AIO::get_segment_no_from_slot(array, slot));
    }
  } else {
    ut_error;
  }

#ifdef WIN_ASYNC_IO
  if (srv_use_native_aio) {
    if ((ret && slot->len == slot->n_bytes) ||
        (!ret && GetLastError() == ERROR_IO_PENDING)) {
      /* AIO was queued successfully! */

      if (aio_mode == AIO_mode::SYNC) {
        IORequest dummy_type;
        void *dummy_mess2;
        struct fil_node_t *dummy_mess1;

        /* We want a synchronous i/o operation on a
        file where we also use async i/o: in Windows
        we must use the same wait mechanism as for
        async i/o */

        return (os_aio_windows_handler(ULINT_UNDEFINED, slot->pos, &dummy_mess1,
                                       &dummy_mess2, &dummy_type));
      }

      return (DB_SUCCESS);
    }

    goto err_exit;
  }
#endif /* WIN_ASYNC_IO */

  /* AIO request was queued successfully! */
  return (DB_SUCCESS);

#if defined LINUX_NATIVE_AIO || defined WIN_ASYNC_IO
err_exit:
#endif /* LINUX_NATIVE_AIO || WIN_ASYNC_IO */

  array->release_with_mutex(slot);

  if (os_file_handle_error(name, type.is_read() ? "aio read" : "aio write")) {
    goto try_again;
  }

  return (DB_IO_ERROR);
}

/** Simulated AIO handler for reaping IO requests */
class SimulatedAIOHandler {
 public:
  /** Constructor
  @param[in,out]	array	The AIO array
  @param[in]	segment	Local segment in the array */
  SimulatedAIOHandler(AIO *array, ulint segment)
      : m_oldest(),
        m_n_elems(),
        m_lowest_offset(IB_UINT64_MAX),
        m_array(array),
        m_n_slots(),
        m_segment(segment),
        m_ptr(),
        m_buf() {
    ut_ad(m_segment < 100);

    m_slots.resize(OS_AIO_MERGE_N_CONSECUTIVE);
  }

  /** Destructor */
  ~SimulatedAIOHandler() {
    if (m_ptr != NULL) {
      ut_free(m_ptr);
    }
  }

  /** Reset the state of the handler
  @param[in]	n_slots	Number of pending AIO operations supported */
  void init(ulint n_slots) {
    m_oldest = 0;
    m_n_elems = 0;
    m_n_slots = n_slots;
    m_lowest_offset = IB_UINT64_MAX;

    if (m_ptr != NULL) {
      ut_free(m_ptr);
      m_ptr = m_buf = NULL;
    }

    m_slots[0] = NULL;
  }

  /** Check if there is a slot for which the i/o has already been done
  @param[out]	n_reserved	Number of reserved slots
  @return the first completed slot that is found. */
  Slot *check_completed(ulint *n_reserved) {
    ulint offset = m_segment * m_n_slots;

    *n_reserved = 0;

    Slot *slot;

    slot = m_array->at(offset);

    for (ulint i = 0; i < m_n_slots; ++i, ++slot) {
      if (slot->is_reserved) {
        if (slot->io_already_done) {
          ut_a(slot->is_reserved);

          return (slot);
        }

        ++*n_reserved;
      }
    }

    return (NULL);
  }

  /** If there are at least 2 seconds old requests, then pick the
  oldest one to prevent starvation.  If several requests have the
  same age, then pick the one at the lowest offset.
  @return true if request was selected */
  bool select() {
    if (!select_oldest()) {
      return (select_lowest_offset());
    }

    return (true);
  }

  /** Check if there are several consecutive blocks
  to read or write. Merge them if found. */
  void merge() {
    /* if m_n_elems != 0, then we have assigned
    something valid to consecutive_ios[0] */
    ut_ad(m_n_elems != 0);
    ut_ad(first_slot() != NULL);

    Slot *slot = first_slot();

    while (!merge_adjacent(slot)) {
      /* No op */
    }
  }

  /** We have now collected n_consecutive I/O requests
  in the array; allocate a single buffer which can hold
  all data, and perform the I/O
  @return the length of the buffer */
  ulint allocate_buffer() MY_ATTRIBUTE((warn_unused_result)) {
    ulint len;
    Slot *slot = first_slot();

    ut_ad(m_ptr == NULL);

    if (slot->type.is_read() && m_n_elems > 1) {
      len = 0;

      for (ulint i = 0; i < m_n_elems; ++i) {
        len += m_slots[i]->len;
      }

      m_ptr = static_cast<byte *>(ut_malloc_nokey(len + UNIV_PAGE_SIZE));

      m_buf = static_cast<byte *>(ut_align(m_ptr, UNIV_PAGE_SIZE));

    } else {
      len = first_slot()->len;
      m_buf = first_slot()->buf;
    }

    return (len);
  }

  /** We have to compress the individual pages and punch
  holes in them on a page by page basis when writing to
  tables that can be compresed at the IO level.
  @param[in]	len		Value returned by allocate_buffer */
  void copy_to_buffer(ulint len) {
    Slot *slot = first_slot();

    if (len > slot->len && slot->type.is_write()) {
      byte *ptr = m_buf;

      ut_ad(ptr != slot->buf);

      /* Copy the buffers to the combined buffer */
      for (ulint i = 0; i < m_n_elems; ++i) {
        slot = m_slots[i];

        memmove(ptr, slot->buf, slot->len);

        ptr += slot->len;
      }
    }
  }

  /** Do the I/O with ordinary, synchronous i/o functions: */
  void io() {
    if (first_slot()->type.is_write()) {
      for (ulint i = 0; i < m_n_elems; ++i) {
        write(m_slots[i]);
      }

    } else {
      for (ulint i = 0; i < m_n_elems; ++i) {
        read(m_slots[i]);
      }
    }
  }

  /** Do the decompression of the pages read in */
  void io_complete() {
    // Note: For non-compressed tables. Not required
    // for correctness.
  }

  /** Mark the i/os done in slots */
  void done() {
    for (ulint i = 0; i < m_n_elems; ++i) {
      m_slots[i]->io_already_done = true;
    }
  }

  /** @return the first slot in the consecutive array */
  Slot *first_slot() MY_ATTRIBUTE((warn_unused_result)) {
    ut_a(m_n_elems > 0);

    return (m_slots[0]);
  }

  /** Wait for I/O requests
  @param[in]	global_segment	The global segment
  @param[in,out]	event		Wait on event if no active requests
  @return the number of slots */
  ulint check_pending(ulint global_segment, os_event_t event)
      MY_ATTRIBUTE((warn_unused_result));

 private:
  /** Do the file read
  @param[in,out]	slot		Slot that has the IO context */
  void read(Slot *slot) {
    dberr_t err = os_file_read_func(slot->type, slot->file.m_file, slot->ptr,
                                    slot->offset, slot->len);
    ut_a(err == DB_SUCCESS);
  }

  /** Do the file write
  @param[in,out]	slot		Slot that has the IO context */
  void write(Slot *slot) {
    dberr_t err = os_file_write_func(slot->type, slot->name, slot->file.m_file,
                                     slot->ptr, slot->offset, slot->len);
    ut_a(err == DB_SUCCESS || err == DB_IO_NO_PUNCH_HOLE);
  }

  /** @return true if the slots are adjacent and can be merged */
  bool adjacent(const Slot *s1, const Slot *s2) const {
    return (s1 != s2 && s1->file.m_file == s2->file.m_file &&
            s2->offset == s1->offset + s1->len && s1->type == s2->type);
  }

  /** @return true if merge limit reached or no adjacent slots found. */
  bool merge_adjacent(Slot *&current) {
    Slot *slot;
    ulint offset = m_segment * m_n_slots;

    slot = m_array->at(offset);

    for (ulint i = 0; i < m_n_slots; ++i, ++slot) {
      if (slot->is_reserved && adjacent(current, slot)) {
        current = slot;

        /* Found a consecutive i/o request */

        m_slots[m_n_elems] = slot;

        ++m_n_elems;

        return (m_n_elems >= m_slots.capacity());
      }
    }

    return (true);
  }

  /** There were no old requests. Look for an I/O request at the lowest
  offset in the array (we ignore the high 32 bits of the offset in these
  heuristics) */
  bool select_lowest_offset() {
    ut_ad(m_n_elems == 0);

    ulint offset = m_segment * m_n_slots;

    m_lowest_offset = IB_UINT64_MAX;

    for (ulint i = 0; i < m_n_slots; ++i) {
      Slot *slot;

      slot = m_array->at(i + offset);

      if (slot->is_reserved && slot->offset < m_lowest_offset) {
        /* Found an i/o request */
        m_slots[0] = slot;

        m_n_elems = 1;

        m_lowest_offset = slot->offset;
      }
    }

    return (m_n_elems > 0);
  }

  /** Select the slot if it is older than the current oldest slot.
  @param[in]	slot		The slot to check */
  void select_if_older(Slot *slot) {
    ulint age;

    age = (ulint)difftime(ut_time(), slot->reservation_time);

    if ((age >= 2 && age > m_oldest) ||
        (age >= 2 && age == m_oldest && slot->offset < m_lowest_offset)) {
      /* Found an i/o request */
      m_slots[0] = slot;

      m_n_elems = 1;

      m_oldest = age;

      m_lowest_offset = slot->offset;
    }
  }

  /** Select th oldest slot in the array
  @return true if oldest slot found */
  bool select_oldest() {
    ut_ad(m_n_elems == 0);

    Slot *slot;
    ulint offset = m_n_slots * m_segment;

    slot = m_array->at(offset);

    for (ulint i = 0; i < m_n_slots; ++i, ++slot) {
      if (slot->is_reserved) {
        select_if_older(slot);
      }
    }

    return (m_n_elems > 0);
  }

  typedef std::vector<Slot *> slots_t;

 private:
  ulint m_oldest;
  ulint m_n_elems;
  os_offset_t m_lowest_offset;

  AIO *m_array;
  ulint m_n_slots;
  ulint m_segment;

  slots_t m_slots;

  byte *m_ptr;
  byte *m_buf;
};

/** Wait for I/O requests
@return the number of slots */
ulint SimulatedAIOHandler::check_pending(ulint global_segment,
                                         os_event_t event) {
/* NOTE! We only access constant fields in os_aio_array.
Therefore we do not have to acquire the protecting mutex yet */

#ifndef UNIV_HOTBACKUP
  ut_ad(os_aio_validate_skip());
#endif /* !UNIV_HOTBACKUP */

  ut_ad(m_segment < m_array->get_n_segments());

  /* Look through n slots after the segment * n'th slot */

  if (AIO::is_read(m_array) && os_aio_recommend_sleep_for_read_threads) {
    /* Give other threads chance to add several
    I/Os to the array at once. */

    srv_set_io_thread_op_info(global_segment, "waiting for i/o request");

    os_event_wait(event);

    return (0);
  }

  return (m_array->slots_per_segment());
}

/** Does simulated AIO. This function should be called by an i/o-handler
thread.

@param[in]	global_segment	The number of the segment in the aio arrays to
                                wait for; segment 0 is the ibuf i/o thread,
                                segment 1 the log i/o thread, then follow the
                                non-ibuf read threads, and as the last are the
                                non-ibuf write threads
@param[out]	m1		the messages passed with the AIO request; note
                                that also in the case where the AIO operation
                                failed, these output parameters are valid and
                                can be used to restart
                                the operation, for example
@param[out]	m2		Callback argument
@param[in]	type		IO context
@return DB_SUCCESS or error code */
static dberr_t os_aio_simulated_handler(ulint global_segment, fil_node_t **m1,
                                        void **m2, IORequest *type) {
  Slot *slot;
  AIO *array;
  ulint segment;
  os_event_t event = os_aio_segment_wait_events[global_segment];

  segment = AIO::get_array_and_local_segment(&array, global_segment);

  SimulatedAIOHandler handler(array, segment);

  for (;;) {
    srv_set_io_thread_op_info(global_segment, "looking for i/o requests (a)");

    ulint n_slots = handler.check_pending(global_segment, event);

    if (n_slots == 0) {
      continue;
    }

    handler.init(n_slots);

    srv_set_io_thread_op_info(global_segment, "looking for i/o requests (b)");

    array->acquire();

    ulint n_reserved;

    slot = handler.check_completed(&n_reserved);

    if (slot != NULL) {
      break;

    } else if (n_reserved == 0
#ifndef UNIV_HOTBACKUP
               && !buf_page_cleaner_is_active &&
               srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS
#endif /* !UNIV_HOTBACKUP */
    ) {

      /* There is no completed request. If there
      are no pending request at all, and the system
      is being shut down, exit. */

      array->release();

      *m1 = NULL;

      *m2 = NULL;

      return (DB_SUCCESS);

    } else if (handler.select()) {
      break;
    }

    /* No I/O requested at the moment */

    srv_set_io_thread_op_info(global_segment, "resetting wait event");

    /* We wait here until tbere are more IO requests
    for this segment. */

    os_event_reset(event);

    array->release();

    srv_set_io_thread_op_info(global_segment, "waiting for i/o request");

    os_event_wait(event);
  }

  /** Found a slot that has already completed its IO */

  if (slot == NULL) {
    /* Merge adjacent requests */
    handler.merge();

    /* Check if there are several consecutive blocks
    to read or write */

    srv_set_io_thread_op_info(global_segment, "consecutive i/o requests");

    // Note: We don't support write combining for simulated AIO.
    // ulint	total_len = handler.allocate_buffer();

    /* We release the array mutex for the time of the I/O: NOTE that
    this assumes that there is just one i/o-handler thread serving
    a single segment of slots! */

    array->release();

    // Note: We don't support write combining for simulated AIO.
    // handler.copy_to_buffer(total_len);

    srv_set_io_thread_op_info(global_segment, "doing file i/o");

    handler.io();

    srv_set_io_thread_op_info(global_segment, "file i/o done");

    handler.io_complete();

    array->acquire();

    handler.done();

    /* We return the messages for the first slot now, and if there
    were several slots, the messages will be returned with
    subsequent calls of this function */

    slot = handler.first_slot();
  }

  ut_ad(slot->is_reserved);

  *m1 = slot->m1;
  *m2 = slot->m2;

  *type = slot->type;

  array->release(slot);

  array->release();

  return (DB_SUCCESS);
}

/** Get the total number of pending IOs
@return the total number of pending IOs */
ulint AIO::total_pending_io_count() {
  ulint count = s_reads->pending_io_count();

  if (s_writes != NULL) {
    count += s_writes->pending_io_count();
  }

  if (s_ibuf != NULL) {
    count += s_ibuf->pending_io_count();
  }

  if (s_log != NULL) {
    count += s_log->pending_io_count();
  }

  if (s_sync != NULL) {
    count += s_sync->pending_io_count();
  }

  return (count);
}

/** Validates the consistency the aio system.
@return true if ok */
static bool os_aio_validate() {
  /* The methods countds and validates, we ignore the count. */
  AIO::total_pending_io_count();

  return (true);
}

/** Prints pending IO requests per segment of an aio array.
We probably don't need per segment statistics but they can help us
during development phase to see if the IO requests are being
distributed as expected.
@param[in,out]	file		File where to print
@param[in]	segments	Pending IO array */
void AIO::print_segment_info(FILE *file, const ulint *segments) {
  ut_ad(m_n_segments > 0);

  if (m_n_segments > 1) {
    fprintf(file, " [");

    for (ulint i = 0; i < m_n_segments; ++i, ++segments) {
      if (i != 0) {
        fprintf(file, ", ");
      }

      fprintf(file, ULINTPF, *segments);
    }

    fprintf(file, "] ");
  }
}

/** Prints info about the aio array.
@param[in,out]	file		Where to print */
void AIO::print(FILE *file) {
  ulint count = 0;
  ulint n_res_seg[SRV_MAX_N_IO_THREADS];

  mutex_enter(&m_mutex);

  ut_a(!m_slots.empty());
  ut_a(m_n_segments > 0);

  memset(n_res_seg, 0x0, sizeof(n_res_seg));

  for (ulint i = 0; i < m_slots.size(); ++i) {
    Slot &slot = m_slots[i];
    ulint segment = (i * m_n_segments) / m_slots.size();

    if (slot.is_reserved) {
      ++count;

      ++n_res_seg[segment];

      ut_a(slot.len > 0);
    }
  }

  ut_a(m_n_reserved == count);

  print_segment_info(file, n_res_seg);

  mutex_exit(&m_mutex);
}

/** Print all the AIO segments
@param[in,out]	file		Where to print */
void AIO::print_all(FILE *file) {
  s_reads->print(file);

  if (s_writes != NULL) {
    fputs(", aio writes:", file);
    s_writes->print(file);
  }

  if (s_ibuf != NULL) {
    fputs(",\n ibuf aio reads:", file);
    s_ibuf->print(file);
  }

  if (s_log != NULL) {
    fputs(", log i/o's:", file);
    s_log->print(file);
  }

  if (s_sync != NULL) {
    fputs(", sync i/o's:", file);
    s_sync->print(file);
  }
}

/** Prints info of the aio arrays.
@param[in,out]	file		file where to print */
void os_aio_print(FILE *file) {
  time_t current_time;
  double time_elapsed;
  double avg_bytes_read;

#ifndef UNIV_HOTBACKUP
  for (ulint i = 0; i < srv_n_file_io_threads; ++i) {
    fprintf(file, "I/O thread %lu state: %s (%s)", (ulong)i,
            srv_io_thread_op_info[i], srv_io_thread_function[i]);

#ifndef _WIN32
    if (os_event_is_set(os_aio_segment_wait_events[i])) {
      fprintf(file, " ev set");
    }
#endif /* _WIN32 */

    fprintf(file, "\n");
  }
#endif /* !UNIV_HOTBACKUP */

  fputs("Pending normal aio reads:", file);

  AIO::print_all(file);

  putc('\n', file);
  current_time = ut_time();
  time_elapsed = 0.001 + difftime(current_time, os_last_printout);

  fprintf(file,
          "Pending flushes (fsync) log: " ULINTPF
          "; "
          "buffer pool: " ULINTPF "\n" ULINTPF " OS file reads, " ULINTPF
          " OS file writes, " ULINTPF " OS fsyncs\n",
          fil_n_pending_log_flushes, fil_n_pending_tablespace_flushes,
          os_n_file_reads, os_n_file_writes, os_n_fsyncs);

  if (os_n_pending_writes != 0 || os_n_pending_reads != 0) {
    fprintf(file, ULINTPF " pending preads, " ULINTPF " pending pwrites\n",
            os_n_pending_reads, os_n_pending_writes);
  }

  if (os_n_file_reads == os_n_file_reads_old) {
    avg_bytes_read = 0.0;
  } else {
    avg_bytes_read = (double)os_bytes_read_since_printout /
                     (os_n_file_reads - os_n_file_reads_old);
  }

  fprintf(file,
          "%.2f reads/s, %lu avg bytes/read,"
          " %.2f writes/s, %.2f fsyncs/s\n",
          (os_n_file_reads - os_n_file_reads_old) / time_elapsed,
          (ulong)avg_bytes_read,
          (os_n_file_writes - os_n_file_writes_old) / time_elapsed,
          (os_n_fsyncs - os_n_fsyncs_old) / time_elapsed);

  os_n_file_reads_old = os_n_file_reads;
  os_n_file_writes_old = os_n_file_writes;
  os_n_fsyncs_old = os_n_fsyncs;
  os_bytes_read_since_printout = 0;

  os_last_printout = current_time;
}

/** Refreshes the statistics used to print per-second averages. */
void os_aio_refresh_stats() {
  os_n_fsyncs_old = os_n_fsyncs;

  os_bytes_read_since_printout = 0;

  os_n_file_reads_old = os_n_file_reads;

  os_n_file_writes_old = os_n_file_writes;

  os_n_fsyncs_old = os_n_fsyncs;

  os_bytes_read_since_printout = 0;

  os_last_printout = ut_time();
}

/** Checks that all slots in the system have been freed, that is, there are
no pending io operations.
@return true if all free */
bool os_aio_all_slots_free() { return (AIO::total_pending_io_count() == 0); }

#ifdef UNIV_DEBUG
/** Prints all pending IO for the array
@param[in]	file	file where to print */
void AIO::to_file(FILE *file) const {
  acquire();

  fprintf(file, " %lu\n", static_cast<ulong>(m_n_reserved));

  for (ulint i = 0; i < m_slots.size(); ++i) {
    const Slot &slot = m_slots[i];

    if (slot.is_reserved) {
      fprintf(file, "%s IO for %s (offset=" UINT64PF ", size=%lu)\n",
              slot.type.is_read() ? "read" : "write", slot.name, slot.offset,
              slot.len);
    }
  }

  release();
}

/** Print pending IOs for all arrays */
void AIO::print_to_file(FILE *file) {
  fprintf(file, "Pending normal aio reads:");

  s_reads->to_file(file);

  if (s_writes != NULL) {
    fprintf(file, "Pending normal aio writes:");
    s_writes->to_file(file);
  }

  if (s_ibuf != NULL) {
    fprintf(file, "Pending ibuf aio reads:");
    s_ibuf->to_file(file);
  }

  if (s_log != NULL) {
    fprintf(file, "Pending log i/o's:");
    s_log->to_file(file);
  }

  if (s_sync != NULL) {
    fprintf(file, "Pending sync i/o's:");
    s_sync->to_file(file);
  }
}

/** Prints all pending IO
@param[in]	file		File where to print */
void os_aio_print_pending_io(FILE *file) { AIO::print_to_file(file); }

#endif /* UNIV_DEBUG */



/**
@param[in]      type            The encryption type
@return the string representation */
const char *Encryption::to_string(Type type) {
  switch (type) {
    case NONE:
      return ("N");
    case AES:
      return ("Y");
  }

  ut_ad(0);

  return ("<UNKNOWN>");
}

/** Generate random encryption value for key and iv.
@param[in,out]	value	Encryption value */
void Encryption::random_value(byte *value) {
  ut_ad(value != NULL);

  my_rand_buffer(value, ENCRYPTION_KEY_LEN);
}

/** Create new master key for key rotation.
@param[in,out]	master_key	master key */
void Encryption::create_master_key(byte **master_key) {
#ifndef UNIV_HOTBACKUP
  size_t key_len;
  char *key_type = NULL;
  char key_name[ENCRYPTION_MASTER_KEY_NAME_MAX_LEN];

  /* If uuid does not match with current server uuid,
  set uuid as current server uuid. */
  if (strcmp(s_uuid, server_uuid) != 0) {
    memcpy(s_uuid, server_uuid, sizeof(s_uuid) - 1);
  }

  /* Generate new master key */
  snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%s-" ULINTPF,
           ENCRYPTION_MASTER_KEY_PRIFIX, s_uuid, s_master_key_id + 1);

  /* We call key ring API to generate master key here. */
  int ret = my_key_generate(key_name, "AES", nullptr, ENCRYPTION_KEY_LEN);

  /* We call key ring API to get master key here. */
  ret = my_key_fetch(key_name, &key_type, nullptr,
                     reinterpret_cast<void **>(master_key), &key_len);

  if (ret != 0 || *master_key == nullptr) {
    ib::error(ER_IB_MSG_831) << "Encryption can't find master key,"
                             << " please check the keyring plugin is loaded."
                             << " ret=" << ret;

    *master_key = nullptr;
  } else {
    ++s_master_key_id;
  }

  if (key_type != nullptr) {
    my_free(key_type);
  }
#endif /* !UNIV_HOTBACKUP */
}

/** Get master key by key id.
@param[in]	master_key_id	master key id
@param[in]	srv_uuid	uuid of server instance
@param[in,out]	master_key	master key */
void Encryption::get_master_key(ulint master_key_id, char *srv_uuid,
                                byte **master_key) {
  size_t key_len = 0;
  char *key_type = nullptr;
  char key_name[ENCRYPTION_MASTER_KEY_NAME_MAX_LEN];

  memset(key_name, 0x0, sizeof(key_name));

  if (srv_uuid != nullptr) {
    ut_ad(strlen(srv_uuid) > 0);

    snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%s-" ULINTPF,
             ENCRYPTION_MASTER_KEY_PRIFIX, srv_uuid, master_key_id);
  } else {
    /* For compitable with 5.7.11, we need to get master key with
    server id. */

    snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%lu-" ULINTPF,
             ENCRYPTION_MASTER_KEY_PRIFIX, server_id, master_key_id);
  }

#ifndef UNIV_HOTBACKUP
  /* We call key ring API to get master key here. */
  int ret = my_key_fetch(key_name, &key_type, nullptr,
                         reinterpret_cast<void **>(master_key), &key_len);
#else  /* !UNIV_HOTBACKUP */
  /* We call MEB to get master key here. */
  int ret = meb_key_fetch(key_name, &key_type, NULL,
                          reinterpret_cast<void **>(master_key), &key_len);
#endif /* !UNIV_HOTBACKUP */

  if (key_type != nullptr) {
    my_free(key_type);
  }

  if (ret != 0) {
    *master_key = nullptr;

    ib::error(ER_IB_MSG_832) << "Encryption can't find master key,"
                             << " please check the keyring plugin is loaded.";
  }

#ifdef UNIV_ENCRYPT_DEBUG
  if (ret == 0 && *master_key != nullptr) {
    std::ostringstream msg;

    ut_print_buf(msg, *master_key, key_len);

    ib::info(ER_IB_MSG_833)
        << "Fetched master key: " << master_key_id << "{" << msg.str() << "}";
  }
#endif /* UNIV_ENCRYPT_DEBUG */
}

/** Current master key id */
ulint Encryption::s_master_key_id = 0;

/** Current uuid of server instance */
char Encryption::s_uuid[ENCRYPTION_SERVER_UUID_LEN + 1] = {0};

/** Get current master key and master key id
@param[in,out]	master_key_id	master key id
@param[in,out]	master_key	master key */
void Encryption::get_master_key(ulint *master_key_id, byte **master_key) {
#ifndef UNIV_HOTBACKUP
  int ret;
  size_t key_len;
  char *key_type = nullptr;
  char key_name[ENCRYPTION_MASTER_KEY_NAME_MAX_LEN];
  extern ib_mutex_t master_key_id_mutex;
  bool key_id_locked = false;

  if (s_master_key_id == 0) {
    mutex_enter(&master_key_id_mutex);
    key_id_locked = true;
  }

  memset(key_name, 0x0, sizeof(key_name));

  if (s_master_key_id == 0) {
    ut_ad(strlen(server_uuid) > 0);
    memset(s_uuid, 0x0, sizeof(s_uuid));

    /* If m_master_key is 0, means there's no encrypted
    tablespace, we need to generate the first master key,
    and store it to key ring. */
    memcpy(s_uuid, server_uuid, sizeof(s_uuid) - 1);

    /* Prepare the server s_uuid. */
    snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%s-1",
             ENCRYPTION_MASTER_KEY_PRIFIX, s_uuid);

    /* We call key ring API to generate master key here. */
    ret = my_key_generate(key_name, "AES", nullptr, ENCRYPTION_KEY_LEN);

    /* We call key ring API to get master key here. */
    ret = my_key_fetch(key_name, &key_type, nullptr,
                       reinterpret_cast<void **>(master_key), &key_len);

    if (ret == 0 && *master_key != nullptr) {
      ++s_master_key_id;
      *master_key_id = s_master_key_id;
    }
#ifdef UNIV_ENCRYPT_DEBUG
    if (ret == 0 && *master_key != nullptr) {
      std::ostringstream msg;

      ut_print_buf(msg, *master_key, key_len);

      ib::info(ER_IB_MSG_834)
          << "Generated new master key: {" << msg.str() << "}";
    }
#endif /* UNIV_ENCRYPT_DEBUG */
  } else {
    *master_key_id = s_master_key_id;

    snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%s-" ULINTPF,
             ENCRYPTION_MASTER_KEY_PRIFIX, s_uuid, *master_key_id);

    /* We call key ring API to get master key here. */
    ret = my_key_fetch(key_name, &key_type, nullptr,
                       reinterpret_cast<void **>(master_key), &key_len);

    /* For compitability with 5.7.11, we need to try to get master
    key with server id when get master key with server uuid
    failure. */
    if (ret != 0 || *master_key == nullptr) {
      if (key_type != nullptr) {
        my_free(key_type);
      }

      snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%lu-" ULINTPF,
               ENCRYPTION_MASTER_KEY_PRIFIX, server_id, *master_key_id);

      ret = my_key_fetch(key_name, &key_type, nullptr,
                         reinterpret_cast<void **>(master_key), &key_len);
    }

#ifdef UNIV_ENCRYPT_DEBUG
    if (ret == 0 && *master_key != nullptr) {
      std::ostringstream msg;

      ut_print_buf(msg, *master_key, key_len);

      ib::info(ER_IB_MSG_835) << "Fetched master key: " << *master_key_id
                              << ": {" << msg.str() << "}";
    }
#endif /* UNIV_ENCRYPT_DEBUG */
  }

  if (ret != 0) {
    *master_key = nullptr;
    ib::error(ER_IB_MSG_836) << "Encryption can't find master key, please check"
                             << " the keyring plugin is loaded.";
  }

  if (key_type != nullptr) {
    my_free(key_type);
  }

  if (key_id_locked) {
    mutex_exit(&master_key_id_mutex);
  }

#endif /* !UNIV_HOTBACKUP */
}

/** Fill the encryption information.
@param[in]	key		encryption key
@param[in]	iv		encryption iv
@param[in,out]	encrypt_info	encryption information
@param[in]	is_boot		if it's for bootstrap
@return true if success */
bool Encryption::fill_encryption_info(byte *key, byte *iv, byte *encrypt_info,
                                      bool is_boot) {
  byte *master_key = nullptr;
  ulint master_key_id;
  bool is_default_master_key = false;

  /* Get master key from key ring. For bootstrap, we use a default
  master key which master_key_id is 0. */
  if (is_boot
#ifndef UNIV_HOTBACKUP
      || (strlen(server_uuid) == 0)
#endif
  ) {
    master_key_id = 0;

    master_key = static_cast<byte *>(ut_zalloc_nokey(ENCRYPTION_KEY_LEN));

    ut_ad(ENCRYPTION_KEY_LEN >= sizeof(ENCRYPTION_DEFAULT_MASTER_KEY));

    strcpy(reinterpret_cast<char *>(master_key), ENCRYPTION_DEFAULT_MASTER_KEY);
    is_default_master_key = true;
  } else {
    get_master_key(&master_key_id, &master_key);

    if (master_key == nullptr) {
      return (false);
    }
  }

  memset(encrypt_info, 0, ENCRYPTION_INFO_SIZE);

  /* Use the new master key to encrypt the key. */
  ut_ad(encrypt_info != nullptr);
  auto ptr = encrypt_info;

  memcpy(ptr, ENCRYPTION_KEY_MAGIC_V3, ENCRYPTION_MAGIC_SIZE);

  ptr += ENCRYPTION_MAGIC_SIZE;

  /* Write master key id. */
  mach_write_to_4(ptr, master_key_id);
  ptr += sizeof(uint32);

  /* Write server uuid. */
  memcpy(reinterpret_cast<char *>(ptr), s_uuid, sizeof(s_uuid));
  ptr += sizeof(s_uuid) - 1;

  byte key_info[ENCRYPTION_KEY_LEN * 2];

  memset(key_info, 0x0, sizeof(key_info));

  memcpy(key_info, key, ENCRYPTION_KEY_LEN);

  memcpy(key_info + ENCRYPTION_KEY_LEN, iv, ENCRYPTION_KEY_LEN);

  /* Encrypt key and iv. */
  auto elen =
      my_aes_encrypt(key_info, sizeof(key_info), ptr, master_key,
                     ENCRYPTION_KEY_LEN, my_aes_256_ecb, nullptr, false);

  if (elen == MY_AES_BAD_DATA) {
    my_free(master_key);
    return (false);
  }

  ptr += sizeof(key_info);

  /* Write checksum bytes. */
  auto crc = ut_crc32(key_info, sizeof(key_info));

  mach_write_to_4(ptr, crc);

  if (is_default_master_key) {
    ut_free(master_key);
  } else {
    my_free(master_key);
  }

  return (true);
}

/** Get master key from encryption information
@param[in]	encrypt_info	encryption information
@param[in]	version		version of encryption information
@param[in,out]	m_key_id	master key id
@param[in,out]	srv_uuid	server uuid
@param[in,out]	master_key	master key
@return position after master key id or uuid, or the old position
if can't get the master key. */
byte *Encryption::get_master_key_from_info(byte *encrypt_info, Version version,
                                           uint32_t *m_key_id, char *srv_uuid,
                                           byte **master_key) {
  byte *ptr;
  uint32 key_id;

  ptr = encrypt_info;
  *m_key_id = 0;

  /* Get master key id. */
  key_id = mach_read_from_4(ptr);
  ptr += sizeof(uint32);

  /* Handle different version encryption information. */
  switch (version) {
    case ENCRYPTION_VERSION_1:
      /* For version 1, it's possible master key id
      occupied 8 bytes. */
      if (mach_read_from_4(ptr) == 0) {
        ptr += sizeof(uint32);
      }

      get_master_key(key_id, nullptr, master_key);
      if (*master_key == nullptr) {
        return (encrypt_info);
      }

      *m_key_id = key_id;
      return (ptr);

    case ENCRYPTION_VERSION_2:
      /* For version 2, it's also possible master key id
      occupied 8 bytes. */
      if (mach_read_from_4(ptr) == 0) {
        ptr += sizeof(uint32);
      }

      /* Get server uuid. */
      memset(srv_uuid, 0, ENCRYPTION_SERVER_UUID_LEN + 1);
      memcpy(srv_uuid, ptr, ENCRYPTION_SERVER_UUID_LEN);

      ut_ad(strlen(srv_uuid) != 0);
      ptr += ENCRYPTION_SERVER_UUID_LEN;

      /* Get master key. */
      get_master_key(key_id, srv_uuid, master_key);
      if (*master_key == nullptr) {
        return (encrypt_info);
      }

      *m_key_id = key_id;
      break;

    case ENCRYPTION_VERSION_3:
      /* Get server uuid. */
      memset(srv_uuid, 0, ENCRYPTION_SERVER_UUID_LEN + 1);
      memcpy(srv_uuid, ptr, ENCRYPTION_SERVER_UUID_LEN);

      ptr += ENCRYPTION_SERVER_UUID_LEN;

      if (key_id == 0) {
        /* When key_id is 0, which means it's the
        default master key for bootstrap. */
        *master_key = static_cast<byte *>(ut_zalloc_nokey(ENCRYPTION_KEY_LEN));
        memcpy(*master_key, ENCRYPTION_DEFAULT_MASTER_KEY,
               strlen(ENCRYPTION_DEFAULT_MASTER_KEY));
        *m_key_id = 0;
      } else {
        ut_ad(strlen(srv_uuid) != 0);

        /* Get master key. */
        get_master_key(key_id, srv_uuid, master_key);
        if (*master_key == nullptr) {
          return (encrypt_info);
        }

        *m_key_id = key_id;
      }
      break;
  }

  ut_ad(*master_key != nullptr);

  return (ptr);
}

/** Decoding the encryption info from the first page of a tablespace.
@param[in,out]	key		key
@param[in,out]	iv		iv
@param[in]	encryption_info	encryption info
@return true if success */
bool Encryption::decode_encryption_info(byte *key, byte *iv,
                                        byte *encryption_info) {
  byte *ptr;
  byte *master_key = nullptr;
  uint32 m_key_id;
  byte key_info[ENCRYPTION_KEY_LEN * 2];
  ulint crc1;
  ulint crc2;
  char srv_uuid[ENCRYPTION_SERVER_UUID_LEN + 1];
  Version version;
#ifdef UNIV_ENCRYPT_DEBUG
  const byte *data;
  ulint i;
#endif

  ptr = encryption_info;

  /* For compatibility with 5.7.11, we need to handle the
  encryption information which created in this old version. */
  if (memcmp(ptr, ENCRYPTION_KEY_MAGIC_V1, ENCRYPTION_MAGIC_SIZE) == 0) {
    version = ENCRYPTION_VERSION_1;
  } else if (memcmp(ptr, ENCRYPTION_KEY_MAGIC_V2, ENCRYPTION_MAGIC_SIZE) == 0) {
    version = ENCRYPTION_VERSION_2;
  } else if (memcmp(ptr, ENCRYPTION_KEY_MAGIC_V3, ENCRYPTION_MAGIC_SIZE) == 0) {
    version = ENCRYPTION_VERSION_3;
  } else {
    /* We don't report an error during recovery, since the
    encryption info maybe hasn't writen into datafile when
    the table is newly created. */
    if (recv_recovery_is_on()) {
      return (true);
    }

    ib::error(ER_IB_MSG_837) << "Failed to decrypt encryption information,"
                             << " found unexpected version of it!";
    return (false);
  }

  ptr += ENCRYPTION_MAGIC_SIZE;

  /* Get master key by key id. */
  ptr =
      get_master_key_from_info(ptr, version, &m_key_id, srv_uuid, &master_key);

  /* If can't find the master key, return failure. */
  if (master_key == nullptr) {
    return (false);
  }

#ifdef UNIV_ENCRYPT_DEBUG
  {
    std::ostringstream msg;

    ut_print_buf_hex(msg, master_key, ENCRYPTION_KEY_LEN);

    ib::info(ER_IB_MSG_838)
        << "Key ID: " << key_id << " hex: {" << msg.str() << "}";
  }
#endif /* UNIV_ENCRYPT_DEBUG */

  /* Decrypt tablespace key and iv. */
  auto len = my_aes_decrypt(ptr, sizeof(key_info), key_info, master_key,
                            ENCRYPTION_KEY_LEN, my_aes_256_ecb, nullptr, false);

  if (m_key_id == 0) {
    ut_free(master_key);
  } else {
    my_free(master_key);
  }

  /* If decryption failed, return error. */
  if (len == MY_AES_BAD_DATA) {
    return (false);
  }

  /* Check checksum bytes. */
  ptr += sizeof(key_info);

  crc1 = mach_read_from_4(ptr);
  crc2 = ut_crc32(key_info, sizeof(key_info));

  if (crc1 != crc2) {
    ib::error(ER_IB_MSG_839)
        << "Failed to decrypt encryption information,"
        << " please check whether key file has been changed!";

    return (false);
  }

  /* Get tablespace key */
  memcpy(key, key_info, ENCRYPTION_KEY_LEN);

  /* Get tablespace iv */
  memcpy(iv, key_info + ENCRYPTION_KEY_LEN, ENCRYPTION_KEY_LEN);

#ifdef UNIV_ENCRYPT_DEBUG
  {
    std::ostringstream msg;

    ut_print_buf_hex(msg, key, ENCRYPTION_KEY_LEN);

    ib::info(ER_IB_MSG_840) << "Key: {" << msg.str() << "}";
  }
  {
    std::ostringstream msg;

    ut_print_buf_hex(msg, iv, ENCRYPTION_KEY_LEN);
    ib::info(ER_IB_MSG_841) << "IV: {" << msg.str() << "}";
  }
#endif /* UNIV_ENCRYPT_DEBUG */

  if (s_master_key_id < m_key_id) {
    s_master_key_id = m_key_id;
    memcpy(s_uuid, srv_uuid, sizeof(s_uuid) - 1);
  }

  return (true);
}

/** Check if page is encrypted page or not
@param[in]	page	page which need to check
@return true if it is a encrypted page */
bool Encryption::is_encrypted_page(const byte *page) {
  ulint page_type = mach_read_from_2(page + FIL_PAGE_TYPE);

  return (page_type == FIL_PAGE_ENCRYPTED ||
          page_type == FIL_PAGE_COMPRESSED_AND_ENCRYPTED ||
          page_type == FIL_PAGE_ENCRYPTED_RTREE);
}

/** Check if redo log block is encrypted block or not
@param[in]	block	log block to check
@return true if it is an encrypted block */
bool Encryption::is_encrypted_log(const byte *block) {
  return (log_block_get_encrypt_bit(block));
}

/** Encrypt the redo log block.
@param[in]	type		IORequest
@param[in]	src_ptr		log block which need to encrypt
@param[in,out]	dst_ptr		destination area
@return true if success. */
bool Encryption::encrypt_log_block(const IORequest &type, byte *src_ptr,
                                   byte *dst_ptr) {
  ulint len = 0;
  ulint data_len;
  ulint main_len;
  ulint remain_len;
  byte remain_buf[MY_AES_BLOCK_SIZE * 2];

#ifdef UNIV_ENCRYPT_DEBUG
  {
    std::ostringstream msg;

    ut_print_buf_hex(msg, src_ptr, OS_FILE_LOG_BLOCK_SIZE);

    ib::info(ER_IB_MSG_842)
        << "Encrypting block: " << log_block_get_hdr_no(src_ptr) << "{"
        << msg.str() << "}";
  }
#endif /* UNIV_ENCRYPT_DEBUG */

  /* This is data size which need to encrypt. */
  data_len = OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_HDR_SIZE;
  main_len = (data_len / MY_AES_BLOCK_SIZE) * MY_AES_BLOCK_SIZE;
  remain_len = data_len - main_len;

  /* Encrypt the block. */
  /* Copy the header as is. */
  memmove(dst_ptr, src_ptr, LOG_BLOCK_HDR_SIZE);
  ut_ad(memcmp(src_ptr, dst_ptr, LOG_BLOCK_HDR_SIZE) == 0);

  switch (m_type) {
    case Encryption::NONE:
      ut_error;

    case Encryption::AES: {
      ut_ad(m_klen == ENCRYPTION_KEY_LEN);

      auto elen = my_aes_encrypt(
          src_ptr + LOG_BLOCK_HDR_SIZE, static_cast<uint32>(main_len),
          dst_ptr + LOG_BLOCK_HDR_SIZE,
          reinterpret_cast<unsigned char *>(m_key), static_cast<uint32>(m_klen),
          my_aes_256_cbc, reinterpret_cast<unsigned char *>(m_iv), false);

      if (elen == MY_AES_BAD_DATA) {
        return (false);
      }

      len = static_cast<ulint>(elen);
      ut_ad(len == main_len);

      /* Copy remain bytes. */
      memcpy(dst_ptr + LOG_BLOCK_HDR_SIZE + len,
             src_ptr + LOG_BLOCK_HDR_SIZE + len,
             OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_HDR_SIZE - len);

      /* Encrypt the remain bytes. Since my_aes_encrypt
      request the content which need to encrypt is
      multiple of MY_AES_BLOCK_SIZE, but the block
      content is possiblly not, so, we need to handle
      the tail bytes first. */
      if (remain_len != 0) {
        remain_len = MY_AES_BLOCK_SIZE * 2;

        elen =
            my_aes_encrypt(dst_ptr + LOG_BLOCK_HDR_SIZE + data_len - remain_len,
                           static_cast<uint32>(remain_len), remain_buf,
                           reinterpret_cast<unsigned char *>(m_key),
                           static_cast<uint32>(m_klen), my_aes_256_cbc,
                           reinterpret_cast<unsigned char *>(m_iv), false);

        if (elen == MY_AES_BAD_DATA) {
          return (false);
        }

        memcpy(dst_ptr + LOG_BLOCK_HDR_SIZE + data_len - remain_len, remain_buf,
               remain_len);
      }

      break;
    }

    default:
      ut_error;
  }

#ifdef UNIV_ENCRYPT_DEBUG
  fprintf(stderr, "Encrypted block %lu.\n", log_block_get_hdr_no(dst_ptr));
  ut_print_buf_hex(stderr, dst_ptr, OS_FILE_LOG_BLOCK_SIZE);
  fprintf(stderr, "\n");

  byte *check_buf =
      static_cast<byte *>(ut_malloc_nokey(OS_FILE_LOG_BLOCK_SIZE));
  byte *buf2 = static_cast<byte *>(ut_malloc_nokey(OS_FILE_LOG_BLOCK_SIZE));

  memcpy(check_buf, dst_ptr, OS_FILE_LOG_BLOCK_SIZE);
  dberr_t err = decrypt_log(type, check_buf, OS_FILE_LOG_BLOCK_SIZE, buf2,
                            OS_FILE_LOG_BLOCK_SIZE);
  log_block_set_encrypt_bit(check_buf, true);
  if (err != DB_SUCCESS ||
      memcmp(src_ptr, check_buf, OS_FILE_LOG_BLOCK_SIZE) != 0) {
    ut_print_buf_hex(stderr, src_ptr, OS_FILE_LOG_BLOCK_SIZE);
    ut_print_buf_hex(stderr, check_buf, OS_FILE_LOG_BLOCK_SIZE);
    ut_ad(0);
  }
  ut_free(buf2);
  ut_free(check_buf);
#endif /* UNIV_ENCRYPT_DEBUG */

  /* Set the encrypted flag. */
  log_block_set_encrypt_bit(dst_ptr, true);

  return (true);
}

/** Encrypt the redo log data contents.
@param[in]	type		IORequest
@param[in]	src		page data which need to encrypt
@param[in]	src_len		Size of the source in bytes
@param[in,out]	dst		destination area
@param[in,out]	dst_len		Size of the destination in bytes
@return buffer data, dst_len will have the length of the data */
byte *Encryption::encrypt_log(const IORequest &type, byte *src, ulint src_len,
                              byte *dst, ulint *dst_len) {
  byte *src_ptr = src;
  byte *dst_ptr = dst;

  ut_ad(type.is_log());
  ut_ad(src_len % OS_FILE_LOG_BLOCK_SIZE == 0);
  ut_ad(m_type != Encryption::NONE);

  /* Encrypt the log blocks one by one. */
  while (src_ptr != src + src_len) {
    if (!encrypt_log_block(type, src_ptr, dst_ptr)) {
      *dst_len = src_len;
      ib::error(ER_IB_MSG_843) << " Can't encrypt data of"
                               << " redo log";
      return (src);
    }

    src_ptr += OS_FILE_LOG_BLOCK_SIZE;
    dst_ptr += OS_FILE_LOG_BLOCK_SIZE;
  }

#ifdef UNIV_ENCRYPT_DEBUG
  byte *check_buf = static_cast<byte *>(ut_malloc_nokey(src_len));
  byte *buf2 = static_cast<byte *>(ut_malloc_nokey(src_len));

  memcpy(check_buf, dst, src_len);

  dberr_t err = decrypt_log(type, check_buf, src_len, buf2, src_len);
  if (err != DB_SUCCESS || memcmp(src, check_buf, src_len) != 0) {
    ut_print_buf_hex(stderr, src, src_len);
    ut_print_buf_hex(stderr, check_buf, src_len);
    ut_ad(0);
  }
  ut_free(buf2);
  ut_free(check_buf);
#endif /* UNIV_ENCRYPT_DEBUG */

  return (dst);
}

/** Encrypt the page data contents. Page type can't be
FIL_PAGE_ENCRYPTED, FIL_PAGE_COMPRESSED_AND_ENCRYPTED,
FIL_PAGE_ENCRYPTED_RTREE.
@param[in]	type		IORequest
@param[in]	src		page data which need to encrypt
@param[in]	src_len		Size of the source in bytes
@param[in,out]	dst		destination area
@param[in,out]	dst_len		Size of the destination in bytes
@return buffer data, dst_len will have the length of the data */
byte *Encryption::encrypt(const IORequest &type, byte *src, ulint src_len,
                          byte *dst, ulint *dst_len) {
  ulint len = 0;
  ulint page_type = mach_read_from_2(src + FIL_PAGE_TYPE);
  ulint data_len;
  ulint main_len;
  ulint remain_len;
  byte remain_buf[MY_AES_BLOCK_SIZE * 2];

  /* For encrypting redo log, take another way. */
  ut_ad(!type.is_log());

#ifdef UNIV_ENCRYPT_DEBUG
  ulint space_id = mach_read_from_4(src + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);
  ulint page_no = mach_read_from_4(src + FIL_PAGE_OFFSET);

  fprintf(stderr, "Encrypting page:%lu.%lu len:%lu\n", space_id, page_no,
          src_len);
  ut_print_buf(stderr, m_key, 32);
  ut_print_buf(stderr, m_iv, 32);
#endif /* UNIV_ENCRYPT_DEBUG */

  /* Shouldn't encrypte an already encrypted page. */
  ut_ad(page_type != FIL_PAGE_ENCRYPTED &&
        page_type != FIL_PAGE_COMPRESSED_AND_ENCRYPTED &&
        page_type != FIL_PAGE_ENCRYPTED_RTREE);

  ut_ad(m_type != Encryption::NONE);

  /* This is data size which need to encrypt. */
  data_len = src_len - FIL_PAGE_DATA;
  main_len = (data_len / MY_AES_BLOCK_SIZE) * MY_AES_BLOCK_SIZE;
  remain_len = data_len - main_len;

  /* Only encrypt the data + trailer, leave the header alone */

  switch (m_type) {
    case Encryption::NONE:
      ut_error;

    case Encryption::AES: {
      lint elen;

      ut_ad(m_klen == ENCRYPTION_KEY_LEN);

      elen = my_aes_encrypt(src + FIL_PAGE_DATA, static_cast<uint32>(main_len),
                            dst + FIL_PAGE_DATA,
                            reinterpret_cast<unsigned char *>(m_key),
                            static_cast<uint32>(m_klen), my_aes_256_cbc,
                            reinterpret_cast<unsigned char *>(m_iv), false);

      if (elen == MY_AES_BAD_DATA) {
        ulint page_no = mach_read_from_4(src + FIL_PAGE_OFFSET);
        ulint space_id =
            mach_read_from_4(src + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);
        *dst_len = src_len;
        ib::error(ER_IB_MSG_844)
            << " Can't encrypt data of page,"
            << " page no:" << page_no << " space id:" << space_id;
        return (src);
      }

      len = static_cast<ulint>(elen);
      ut_ad(len == main_len);

      /* Copy remain bytes and page tailer. */
      memcpy(dst + FIL_PAGE_DATA + len, src + FIL_PAGE_DATA + len,
             src_len - FIL_PAGE_DATA - len);

      /* Encrypt the remain bytes. */
      if (remain_len != 0) {
        remain_len = MY_AES_BLOCK_SIZE * 2;

        elen = my_aes_encrypt(dst + FIL_PAGE_DATA + data_len - remain_len,
                              static_cast<uint32>(remain_len), remain_buf,
                              reinterpret_cast<unsigned char *>(m_key),
                              static_cast<uint32>(m_klen), my_aes_256_cbc,
                              reinterpret_cast<unsigned char *>(m_iv), false);

        if (elen == MY_AES_BAD_DATA) {
          ulint page_no = mach_read_from_4(src + FIL_PAGE_OFFSET);
          ulint space_id =
              mach_read_from_4(src + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);

          ib::error(ER_IB_MSG_845)
              << " Can't encrypt data of page,"
              << " page no:" << page_no << " space id:" << space_id;
          *dst_len = src_len;
          return (src);
        }

        memcpy(dst + FIL_PAGE_DATA + data_len - remain_len, remain_buf,
               remain_len);
      }

      break;
    }

    default:
      ut_error;
  }

  /* Copy the header as is. */
  memmove(dst, src, FIL_PAGE_DATA);
  ut_ad(memcmp(src, dst, FIL_PAGE_DATA) == 0);

  /* Add encryption control information. Required for decrypting. */
  if (page_type == FIL_PAGE_COMPRESSED) {
    /* If the page is compressed, we don't need to save the
    original type, since it is done in compression already. */
    mach_write_to_2(dst + FIL_PAGE_TYPE, FIL_PAGE_COMPRESSED_AND_ENCRYPTED);
    ut_ad(memcmp(src + FIL_PAGE_TYPE + 2, dst + FIL_PAGE_TYPE + 2,
                 FIL_PAGE_DATA - FIL_PAGE_TYPE - 2) == 0);
  } else if (page_type == FIL_PAGE_RTREE) {
    /* If the page is R-tree page, we need to save original
    type. */
    mach_write_to_2(dst + FIL_PAGE_TYPE, FIL_PAGE_ENCRYPTED_RTREE);
  } else {
    mach_write_to_2(dst + FIL_PAGE_TYPE, FIL_PAGE_ENCRYPTED);
    mach_write_to_2(dst + FIL_PAGE_ORIGINAL_TYPE_V1, page_type);
  }

#ifdef UNIV_ENCRYPT_DEBUG
  byte *check_buf = static_cast<byte *>(ut_malloc_nokey(src_len));
  byte *buf2 = static_cast<byte *>(ut_malloc_nokey(src_len));

  memcpy(check_buf, dst, src_len);

  dberr_t err = decrypt(type, check_buf, src_len, buf2, src_len);
  if (err != DB_SUCCESS ||
      memcmp(src + FIL_PAGE_DATA, check_buf + FIL_PAGE_DATA,
             src_len - FIL_PAGE_DATA) != 0) {
    ut_print_buf(stderr, src, src_len);
    ut_print_buf(stderr, check_buf, src_len);
    ut_ad(0);
  }
  ut_free(buf2);
  ut_free(check_buf);

  fprintf(stderr, "Encrypted page:%lu.%lu\n", space_id, page_no);
#endif /* UNIV_ENCRYPT_DEBUG */

  *dst_len = src_len;

  return (dst);
}

/** Decrypt the log block.
@param[in]	type		IORequest
@param[in,out]	src		Data read from disk, decrypted data will be
                                copied to this page
@param[in,out]	dst		Scratch area to use for decryption
@return DB_SUCCESS or error code */
dberr_t Encryption::decrypt_log_block(const IORequest &type, byte *src,
                                      byte *dst) {
  ulint data_len;
  ulint main_len;
  ulint remain_len;
  byte remain_buf[MY_AES_BLOCK_SIZE * 2];
  byte *ptr = src;

  /* This is data size which need to encrypt. */
  data_len = OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_HDR_SIZE;
  main_len = (data_len / MY_AES_BLOCK_SIZE) * MY_AES_BLOCK_SIZE;
  remain_len = data_len - main_len;

  ptr += LOG_BLOCK_HDR_SIZE;
  switch (m_type) {
    case Encryption::AES: {
      lint elen;

      /* First decrypt the last 2 blocks data of data, since
      data is no block aligned. */
      if (remain_len != 0) {
        ut_ad(m_klen == ENCRYPTION_KEY_LEN);

        remain_len = MY_AES_BLOCK_SIZE * 2;

        /* Copy the last 2 blocks. */
        memcpy(remain_buf, ptr + data_len - remain_len, remain_len);

        elen = my_aes_decrypt(remain_buf, static_cast<uint32>(remain_len),
                              dst + data_len - remain_len,
                              reinterpret_cast<unsigned char *>(m_key),
                              static_cast<uint32>(m_klen), my_aes_256_cbc,
                              reinterpret_cast<unsigned char *>(m_iv), false);
        if (elen == MY_AES_BAD_DATA) {
          return (DB_IO_DECRYPT_FAIL);
        }

        /* Copy the other data bytes to temp area. */
        memcpy(dst, ptr, data_len - remain_len);
      } else {
        ut_ad(data_len == main_len);

        /* Copy the data bytes to temp area. */
        memcpy(dst, ptr, data_len);
      }

      /* Then decrypt the main data */
      elen = my_aes_decrypt(dst, static_cast<uint32>(main_len), ptr,
                            reinterpret_cast<unsigned char *>(m_key),
                            static_cast<uint32>(m_klen), my_aes_256_cbc,
                            reinterpret_cast<unsigned char *>(m_iv), false);
      if (elen == MY_AES_BAD_DATA) {
        return (DB_IO_DECRYPT_FAIL);
      }

      ut_ad(static_cast<ulint>(elen) == main_len);

      /* Copy the remain bytes. */
      memcpy(ptr + main_len, dst + main_len, data_len - main_len);

      break;
    }

    default:
      ib::error(ER_IB_MSG_846) << "Encryption algorithm support missing: "
                               << Encryption::to_string(m_type);
      return (DB_UNSUPPORTED);
  }

  ptr -= LOG_BLOCK_HDR_SIZE;

#ifdef UNIV_ENCRYPT_DEBUG
  fprintf(stderr, "Decrypted block %lu.\n", log_block_get_hdr_no(ptr));
  ut_print_buf_hex(stderr, ptr, OS_FILE_LOG_BLOCK_SIZE);
  fprintf(stderr, "\n");
#endif

  /* Reset the encrypted flag. */
  log_block_set_encrypt_bit(ptr, false);

  return (DB_SUCCESS);
}

/** Decrypt the log data contents.
@param[in]	type		IORequest
@param[in,out]	src		Data read from disk, decrypted data will be
                                copied to this page
@param[in]	src_len		source data length
@param[in,out]	dst		Scratch area to use for decryption
@param[in]	dst_len		Size of the scratch area in bytes
@return DB_SUCCESS or error code */
dberr_t Encryption::decrypt_log(const IORequest &type, byte *src, ulint src_len,
                                byte *dst, ulint dst_len) {
  Block *block;
  byte *ptr = src;
  dberr_t ret;

  /* Do nothing if it's not a log request. */
  ut_ad(type.is_log());

  /* The caller doesn't know what to expect */
  if (dst == NULL) {
    block = os_alloc_block();
    dst = block->m_ptr;
  } else {
    block = NULL;
  }

  /* Encrypt the log blocks one by one. */
  while (ptr != src + src_len) {
#ifdef UNIV_ENCRYPT_DEBUG
    {
      std::ostringstream msg;

      ut_print_buf_hex(msg, ptr, OS_FILE_LOG_BLOCK_SIZE);

      ib::info(ER_IB_MSG_847)
          << "Decrypting block: " << log_block_get_hdr_no(ptr) << std::endl
          << "data={" << std::endl
          << msg.str << std::endl
          << "}";
    }
#endif /* UNIV_ENCRYPT_DEBUG */

    /* If it's not an encrypted block, skip it. */
    if (!is_encrypted_log(ptr)) {
      ptr += OS_FILE_LOG_BLOCK_SIZE;
      continue;
    }

    /* Decrypt block */
    ret = decrypt_log_block(type, ptr, dst);
    if (ret != DB_SUCCESS) {
      if (block != NULL) {
        os_free_block(block);
      }

      return (ret);
    }

    ptr += OS_FILE_LOG_BLOCK_SIZE;
  }

  if (block != NULL) {
    os_free_block(block);
  }

  return (DB_SUCCESS);
}

/** Decrypt the page data contents. Page type must be FIL_PAGE_ENCRYPTED,
if not then the source contents are left unchanged and DB_SUCCESS is returned.
@param[in]	type		IORequest
@param[in,out]	src		Data read from disk, decrypted data will be
                                copied to this page
@param[in]	src_len		source data length
@param[in,out]	dst		Scratch area to use for decryption
@param[in]	dst_len		Size of the scratch area in bytes
@return DB_SUCCESS or error code */
dberr_t Encryption::decrypt(const IORequest &type, byte *src, ulint src_len,
                            byte *dst, ulint dst_len) {
  ulint data_len;
  ulint main_len;
  ulint remain_len;
  ulint original_type;
  ulint page_type;
  byte remain_buf[MY_AES_BLOCK_SIZE * 2];
  Block *block;

  if (!is_encrypted_page(src) || m_type == Encryption::NONE) {
    /* There is nothing we can do. */
    return (DB_SUCCESS);
  }

  /* For compressed page, we need to get the compressed size
  for decryption */
  page_type = mach_read_from_2(src + FIL_PAGE_TYPE);
  if (page_type == FIL_PAGE_COMPRESSED_AND_ENCRYPTED) {
    src_len = static_cast<uint16_t>(
                  mach_read_from_2(src + FIL_PAGE_COMPRESS_SIZE_V1)) +
              FIL_PAGE_DATA;
    src_len = ut_calc_align(src_len, type.block_size());
  }

#ifdef UNIV_ENCRYPT_DEBUG
  {
    auto space_id = mach_read_from_4(src + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);

    auto page_no = mach_read_from_4(src + FIL_PAGE_OFFSET);

    std::ostringstream msg;

    msg << "key={";
    ut_print_buf(msg, m_key, 32);
    msg << "}" << std::endl << "iv= {";
    ut_print_buf(msg, m_iv, 32);
    msg << "}";

    ib::info(ER_IB_MSG_848) << "Decrypting page: " << space_id << "." << page_no
                            << " len: " << src_len << "\n"
                            << msg.str();
  }
#endif /* UNIV_ENCRYPT_DEBUG */

  original_type =
      static_cast<uint16_t>(mach_read_from_2(src + FIL_PAGE_ORIGINAL_TYPE_V1));

  byte *ptr = src + FIL_PAGE_DATA;

  /* The caller doesn't know what to expect */
  if (dst == NULL) {
    block = os_alloc_block();
    dst = block->m_ptr;
  } else {
    block = NULL;
  }

  data_len = src_len - FIL_PAGE_DATA;
  main_len = (data_len / MY_AES_BLOCK_SIZE) * MY_AES_BLOCK_SIZE;
  remain_len = data_len - main_len;

  switch (m_type) {
    case Encryption::AES: {
      lint elen;

      /* First decrypt the last 2 blocks data of data, since
      data is no block aligned. */
      if (remain_len != 0) {
        ut_ad(m_klen == ENCRYPTION_KEY_LEN);

        remain_len = MY_AES_BLOCK_SIZE * 2;

        /* Copy the last 2 blocks. */
        memcpy(remain_buf, ptr + data_len - remain_len, remain_len);

        elen = my_aes_decrypt(remain_buf, static_cast<uint32>(remain_len),
                              dst + data_len - remain_len,
                              reinterpret_cast<unsigned char *>(m_key),
                              static_cast<uint32>(m_klen), my_aes_256_cbc,
                              reinterpret_cast<unsigned char *>(m_iv), false);
        if (elen == MY_AES_BAD_DATA) {
          if (block != NULL) {
            os_free_block(block);
          }

          return (DB_IO_DECRYPT_FAIL);
        }

        /* Copy the other data bytes to temp area. */
        memcpy(dst, ptr, data_len - remain_len);
      } else {
        ut_ad(data_len == main_len);

        /* Copy the data bytes to temp area. */
        memcpy(dst, ptr, data_len);
      }

      /* Then decrypt the main data */
      elen = my_aes_decrypt(dst, static_cast<uint32>(main_len), ptr,
                            reinterpret_cast<unsigned char *>(m_key),
                            static_cast<uint32>(m_klen), my_aes_256_cbc,
                            reinterpret_cast<unsigned char *>(m_iv), false);
      if (elen == MY_AES_BAD_DATA) {
        if (block != NULL) {
          os_free_block(block);
        }

        return (DB_IO_DECRYPT_FAIL);
      }

      ut_ad(static_cast<ulint>(elen) == main_len);

      /* Copy the remain bytes. */
      memcpy(ptr + main_len, dst + main_len, data_len - main_len);

      break;
    }

    default:
      if (!type.is_dblwr_recover()) {
        ib::error(ER_IB_MSG_849) << "Encryption algorithm support missing: "
                                 << Encryption::to_string(m_type);
      }

      if (block != NULL) {
        os_free_block(block);
      }

      return (DB_UNSUPPORTED);
  }

  /* Restore the original page type. If it's a compressed and
  encrypted page, just reset it as compressed page type, since
  we will do uncompress later. */

  if (page_type == FIL_PAGE_ENCRYPTED) {
    mach_write_to_2(src + FIL_PAGE_TYPE, original_type);
    mach_write_to_2(src + FIL_PAGE_ORIGINAL_TYPE_V1, 0);
  } else if (page_type == FIL_PAGE_ENCRYPTED_RTREE) {
    mach_write_to_2(src + FIL_PAGE_TYPE, FIL_PAGE_RTREE);
  } else {
    ut_ad(page_type == FIL_PAGE_COMPRESSED_AND_ENCRYPTED);
    mach_write_to_2(src + FIL_PAGE_TYPE, FIL_PAGE_COMPRESSED);
  }

  if (block != NULL) {
    os_free_block(block);
  }

#ifdef UNIV_ENCRYPT_DEBUG
  ib::info(ER_IB_MSG_850) << "Decrypted page: " << space_id << "." << page_no;
#endif /* UNIV_ENCRYPT_DEBUG */

  DBUG_EXECUTE_IF("ib_crash_during_decrypt_page", DBUG_SUICIDE(););

  return (DB_SUCCESS);
}

#ifndef UNIV_HOTBACKUP
/** Check if keyring plugin loaded. */
bool Encryption::check_keyring() {
  size_t key_len;
  bool ret = false;
  char *key_type = nullptr;
  char *master_key = nullptr;
  char key_name[ENCRYPTION_MASTER_KEY_NAME_MAX_LEN];

  key_name[sizeof(ENCRYPTION_DEFAULT_MASTER_KEY)] = 0;

  strncpy(key_name, ENCRYPTION_DEFAULT_MASTER_KEY, sizeof(key_name));

  /* We call key ring API to generate master key here. */
  int my_ret = my_key_generate(key_name, "AES", NULL, ENCRYPTION_KEY_LEN);

  /* We call key ring API to get master key here. */
  my_ret = my_key_fetch(key_name, &key_type, nullptr,
                        reinterpret_cast<void **>(&master_key), &key_len);

  if (my_ret != 0) {
    ib::error(ER_IB_MSG_851) << "Check keyring plugin fail, please check the"
                             << " keyring plugin is loaded.";
  } else {
    my_key_remove(key_name, nullptr);
    ret = true;
  }

  if (key_type != nullptr) {
    my_free(key_type);
  }

  if (master_key != nullptr) {
    my_free(master_key);
  }

  return (ret);
}
#endif /* !UNIV_HOTBACKUP */

/** Check if the path is a directory. The file/directory must exist.
@param[in]	path		The path to check
@return true if it is a directory */
bool Dir_Walker::is_directory(const Path &path) {
  os_file_type_t type;
  bool exists;

  if (os_file_status(path.c_str(), &exists, &type)) {
    ut_ad(exists);
    ut_ad(type != OS_FILE_TYPE_MISSING);

    return (type == OS_FILE_TYPE_DIR);
  }

  ut_ad(exists || type == OS_FILE_TYPE_FAILED);
  ut_ad(type != OS_FILE_TYPE_MISSING);

  return (false);
}
