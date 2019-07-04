/***********************************************************************

Copyright (c) 1995, 2018, Oracle and/or its affiliates. All Rights Reserved.
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

/** @file include/os0file.h
 The interface to the operating system file io

 Created 10/21/1995 Heikki Tuuri
 *******************************************************/

#ifndef os0file_h
#define os0file_h

#include <innodb/univ/univ.h>
#include <innodb/assert/assert.h>
#include <innodb/error/dberr_t.h>

#include <innodb/io/Encryption.h>
#include <innodb/io/os_offset_t.h>
#include <innodb/io/os_file_t.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/os_file_size_t.h>
#include <innodb/io/AIO_mode.h>
#include <innodb/io/os_file_type_t.h>
#include <innodb/io/os_file_stat_t.h>
#include <innodb/io/os_file_get_last_error_low.h>
#include <innodb/io/os_file_get_last_error.h>
#include <innodb/io/os_file_handle_error_cond_exit.h>
#include <innodb/io/os_file_handle_error.h>
#include <innodb/io/os_file_handle_error_no_exit.h>
#include <innodb/io/os_file_create_directory.h>
#include <innodb/io/os_file_scan_directory.h>
#include <innodb/io/os_file_is_root.h>
#include <innodb/io/os_file_get_parent_dir.h>
#include <innodb/io/os_file_status_posix.h>
#include <innodb/io/os_file_lock.h>
#include <innodb/io/os_file_create_simple_func.h>
#include <innodb/io/os_file_set_nocache.h>
#include <innodb/io/os_file_set_umask.h>
#include <innodb/io/os_innodb_umask.h>
#include <innodb/io/os_file_rename_func.h>
#include <innodb/io/os_file_close_func.h>
#include <innodb/io/os_file_get_status_posix.h>
#include <innodb/io/os_file_delete_func.h>
#include <innodb/io/os_file_get_size.h>
#include <innodb/io/os_file_create_func.h>
#include <innodb/io/access_type.h>
#include <innodb/io/os_file_create_simple_no_error_handling_func.h>
#include <innodb/io/os_create_block_cache.h>
#include <innodb/io/os_alloc_block.h>
#include <innodb/io/os_free_block.h>
#include <innodb/io/meb_free_block_cache.h>
#include <innodb/io/os_file_compress_page.h>
#include <innodb/io/os_file_fsync_posix.h>
#include <innodb/io/os_file_flush_func.h>
#include <innodb/io/os_file_can_delete.h>
#include <innodb/io/os_file_delete_if_exists_func.h>

#include "my_dbug.h"
#include "my_io.h"
#include "os/file.h"


#ifndef _WIN32
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#else
#include <Strsafe.h>
#include <locale>
#include <string>
#endif /* !_WIN32 */

#include <functional>
#include <stack>

/** File node of a tablespace or the log data space */
struct fil_node_t;

extern bool os_has_said_disk_full;

/** Number of pending read operations */
extern ulint os_n_pending_reads;
/** Number of pending write operations */
extern ulint os_n_pending_writes;

/* Flush after each os_fsync_threshold bytes */
extern unsigned long long os_fsync_threshold;


#ifdef _WIN32

typedef HANDLE os_file_dir_t; /*!< directory stream */

/** We define always WIN_ASYNC_IO, and check at run-time whether
the OS actually supports it: Win 95 does not, NT does. */
#define WIN_ASYNC_IO

/** Use unbuffered I/O */
#define UNIV_NON_BUFFERED_IO


/** Convert a C file descriptor to a native file handle
@param fd file descriptor
@return native file handle */
#define OS_FILE_FROM_FD(fd) (HANDLE) _get_osfhandle(fd)

/** Associates a C file descriptor with an existing native file handle
@param[in]	file	native file handle
@return C file descriptor */
#define OS_FD_FROM_FILE(file) _open_osfhandle((intptr_t)file, _O_RDONLY)

/** Closes the file associated with C file descriptor fd
@param[in]	fd	C file descriptor
@return 0 if success */
#define OS_FILE_CLOSE_FD(fd) _close(fd)

#else /* _WIN32 */


/** Convert a C file descriptor to a native file handle
@param fd file descriptor
@return native file handle */
#define OS_FILE_FROM_FD(fd) fd

/** C file descriptor from an existing native file handle
@param[in]	file	native file handle
@return C file descriptor */
#define OS_FD_FROM_FILE(file) file

/** Closes the file associated with C file descriptor fd
@param[in]	fd	C file descriptor
@return 0 if success */
#define OS_FILE_CLOSE_FD(fd) (os_file_close(fd) ? 0 : -1)

#endif /* _WIN32 */





/** The next value should be smaller or equal to the smallest sector size used
on any disk. A log block is required to be a portion of disk which is written
so that if the start and the end of a block get written to disk, then the
whole block gets written. This should be true even in most cases of a crash:
if this fails for a log block, then it is equivalent to a media failure in the
log. */

#define OS_FILE_LOG_BLOCK_SIZE 512



/** Options for os_file_create_func @{ */









/** Encryption key length */
static const ulint ENCRYPTION_KEY_LEN = 32;

/** Encryption magic bytes size */
static const ulint ENCRYPTION_MAGIC_SIZE = 3;

/** Encryption magic bytes for 5.7.11, it's for checking the encryption
information version. */
static const char ENCRYPTION_KEY_MAGIC_V1[] = "lCA";

/** Encryption magic bytes for 5.7.12+, it's for checking the encryption
information version. */
static const char ENCRYPTION_KEY_MAGIC_V2[] = "lCB";

/** Encryption magic bytes for 8.0.5+, it's for checking the encryption
information version. */
static const char ENCRYPTION_KEY_MAGIC_V3[] = "lCC";

/** Encryption master key prifix */
static const char ENCRYPTION_MASTER_KEY_PRIFIX[] = "INNODBKey";

/** Encryption master key prifix size */
static const ulint ENCRYPTION_MASTER_KEY_PRIFIX_LEN = 9;

/** Encryption master key prifix size */
static const ulint ENCRYPTION_MASTER_KEY_NAME_MAX_LEN = 100;


/** Encryption information total size: magic number + master_key_id +
key + iv + server_uuid + checksum */
static const ulint ENCRYPTION_INFO_SIZE =
    (ENCRYPTION_MAGIC_SIZE + sizeof(uint32) + (ENCRYPTION_KEY_LEN * 2) +
     ENCRYPTION_SERVER_UUID_LEN + sizeof(uint32));

/** Maximum size of Encryption information considering all formats v1, v2 & v3.
 */
static const ulint ENCRYPTION_INFO_MAX_SIZE =
    (ENCRYPTION_INFO_SIZE + sizeof(uint32));

/** Default master key for bootstrap */
static const char ENCRYPTION_DEFAULT_MASTER_KEY[] = "DefaultMasterKey";

/** Default master key id for bootstrap */
static const ulint ENCRYPTION_DEFAULT_MASTER_KEY_ID = 0;

/** (Un)Encryption Operation information size */
static const uint ENCRYPTION_OPERATION_INFO_SIZE = 1;

/** Encryption Progress information size */
static const uint ENCRYPTION_PROGRESS_INFO_SIZE = sizeof(uint);

/** Flag bit to indicate if Encryption/Decryption is in progress */
#define ENCRYPTION_IN_PROGRESS (1 << 0)
#define UNENCRYPTION_IN_PROGRESS (1 << 1)




/** Types for AIO operations @{ */

/** No transformations during read/write, write as is. */
#define IORequestRead IORequest(IORequest::READ)
#define IORequestWrite IORequest(IORequest::WRITE)
#define IORequestLogRead IORequest(IORequest::LOG | IORequest::READ)
#define IORequestLogWrite IORequest(IORequest::LOG | IORequest::WRITE)

/* @} */





/** Win NT does not allow more than 64 */
static const ulint OS_AIO_N_PENDING_IOS_PER_THREAD = 32;



extern ulint os_n_file_reads;
extern ulint os_n_file_writes;
extern ulint os_n_fsyncs;





#ifndef UNIV_HOTBACKUP
/** Create a temporary file. This function is like tmpfile(3), but
the temporary file is created in the given parameter path. If the path
is null then it will create the file in the mysql server configuration
parameter (--tmpdir).
@param[in]	path	location for creating temporary file
@return temporary file handle, or NULL on error */
FILE *os_file_create_tmpfile(const char *path);
#endif /* !UNIV_HOTBACKUP */




#ifdef UNIV_PFS_IO

/* Keys to register InnoDB I/O with performance schema */
extern mysql_pfs_key_t innodb_log_file_key;
extern mysql_pfs_key_t innodb_temp_file_key;
extern mysql_pfs_key_t innodb_arch_file_key;
extern mysql_pfs_key_t innodb_clone_file_key;
extern mysql_pfs_key_t innodb_data_file_key;
extern mysql_pfs_key_t innodb_tablespace_open_file_key;

/* Following four macros are instumentations to register
various file I/O operations with performance schema.
1) register_pfs_file_open_begin() and register_pfs_file_open_end() are
used to register file creation, opening and closing.
2) register_pfs_file_rename_begin() and  register_pfs_file_rename_end()
are used to register file renaming.
3) register_pfs_file_io_begin() and register_pfs_file_io_end() are
used to register actual file read, write and flush
4) register_pfs_file_close_begin() and register_pfs_file_close_end()
are used to register file deletion operations*/
#define register_pfs_file_open_begin(state, locker, key, op, name, src_file, \
                                     src_line)                               \
  do {                                                                       \
    locker = PSI_FILE_CALL(get_thread_file_name_locker)(state, key.m_value,  \
                                                        op, name, &locker);  \
    if (locker != NULL) {                                                    \
      PSI_FILE_CALL(start_file_open_wait)                                    \
      (locker, src_file, static_cast<uint>(src_line));                       \
    }                                                                        \
  } while (0)

#define register_pfs_file_open_end(locker, file, result)              \
  do {                                                                \
    if (locker != NULL) {                                             \
      file.m_psi = PSI_FILE_CALL(end_file_open_wait)(locker, result); \
    }                                                                 \
  } while (0)

#define register_pfs_file_rename_begin(state, locker, key, op, name, src_file, \
                                       src_line)                               \
  register_pfs_file_open_begin(state, locker, key, op, name, src_file,         \
                               static_cast<uint>(src_line))

#define register_pfs_file_rename_end(locker, from, to, result)       \
  do {                                                               \
    if (locker != NULL) {                                            \
      PSI_FILE_CALL(end_file_rename_wait)(locker, from, to, result); \
    }                                                                \
  } while (0)

#define register_pfs_file_close_begin(state, locker, key, op, name, src_file, \
                                      src_line)                               \
  do {                                                                        \
    locker = PSI_FILE_CALL(get_thread_file_name_locker)(state, key.m_value,   \
                                                        op, name, &locker);   \
    if (locker != NULL) {                                                     \
      PSI_FILE_CALL(start_file_close_wait)                                    \
      (locker, src_file, static_cast<uint>(src_line));                        \
    }                                                                         \
  } while (0)

#define register_pfs_file_close_end(locker, result)       \
  do {                                                    \
    if (locker != NULL) {                                 \
      PSI_FILE_CALL(end_file_close_wait)(locker, result); \
    }                                                     \
  } while (0)

#define register_pfs_file_io_begin(state, locker, file, count, op, src_file, \
                                   src_line)                                 \
  do {                                                                       \
    locker =                                                                 \
        PSI_FILE_CALL(get_thread_file_stream_locker)(state, file.m_psi, op); \
    if (locker != NULL) {                                                    \
      PSI_FILE_CALL(start_file_wait)                                         \
      (locker, count, src_file, static_cast<uint>(src_line));                \
    }                                                                        \
  } while (0)

#define register_pfs_file_io_end(locker, count)    \
  do {                                             \
    if (locker != NULL) {                          \
      PSI_FILE_CALL(end_file_wait)(locker, count); \
    }                                              \
  } while (0)

/* Following macros/functions are file I/O APIs that would be performance
schema instrumented if "UNIV_PFS_IO" is defined. They would point to
wrapper functions with performance schema instrumentation in such case.

os_file_create
os_file_create_simple
os_file_create_simple_no_error_handling
os_file_close
os_file_rename
os_aio
os_file_read
os_file_read_no_error_handling
os_file_read_no_error_handling_int_fd
os_file_write

The wrapper functions have the prefix of "innodb_". */

#define os_file_create(key, name, create, purpose, type, read_only, success) \
  pfs_os_file_create_func(key, name, create, purpose, type, read_only,       \
                          success, __FILE__, __LINE__)

#define os_file_create_simple(key, name, create, access, read_only, success) \
  pfs_os_file_create_simple_func(key, name, create, access, read_only,       \
                                 success, __FILE__, __LINE__)

#define os_file_create_simple_no_error_handling(key, name, create_mode,     \
                                                access, read_only, success) \
  pfs_os_file_create_simple_no_error_handling_func(                         \
      key, name, create_mode, access, read_only, success, __FILE__, __LINE__)

#define os_file_close_pfs(file) pfs_os_file_close_func(file, __FILE__, __LINE__)

#define os_aio(type, mode, name, file, buf, offset, n, read_only, message1,    \
               message2)                                                       \
  pfs_os_aio_func(type, mode, name, file, buf, offset, n, read_only, message1, \
                  message2, __FILE__, __LINE__)

#define os_file_read_pfs(type, file, buf, offset, n) \
  pfs_os_file_read_func(type, file, buf, offset, n, __FILE__, __LINE__)

#define os_file_read_first_page_pfs(type, file, buf, n) \
  pfs_os_file_read_first_page_func(type, file, buf, n, __FILE__, __LINE__)

#define os_file_copy_pfs(src, src_offset, dest, dest_offset, size)          \
  pfs_os_file_copy_func(src, src_offset, dest, dest_offset, size, __FILE__, \
                        __LINE__)

#define os_file_read_no_error_handling_pfs(type, file, buf, offset, n, o) \
  pfs_os_file_read_no_error_handling_func(type, file, buf, offset, n, o,  \
                                          __FILE__, __LINE__)

#define os_file_read_no_error_handling_int_fd(type, file, buf, offset, n, o) \
  pfs_os_file_read_no_error_handling_int_fd_func(type, file, buf, offset, n, \
                                                 o, __FILE__, __LINE__)

#define os_file_write_pfs(type, name, file, buf, offset, n) \
  pfs_os_file_write_func(type, name, file, buf, offset, n, __FILE__, __LINE__)

#define os_file_write_int_fd(type, name, file, buf, offset, n)              \
  pfs_os_file_write_int_fd_func(type, name, file, buf, offset, n, __FILE__, \
                                __LINE__)

#define os_file_flush_pfs(file) pfs_os_file_flush_func(file, __FILE__, __LINE__)

#define os_file_rename(key, oldpath, newpath) \
  pfs_os_file_rename_func(key, oldpath, newpath, __FILE__, __LINE__)

#define os_file_delete(key, name) \
  pfs_os_file_delete_func(key, name, __FILE__, __LINE__)

#define os_file_delete_if_exists(key, name, exist) \
  pfs_os_file_delete_if_exists_func(key, name, exist, __FILE__, __LINE__)

/** NOTE! Please use the corresponding macro os_file_create_simple(),
not directly this function!
A performance schema instrumented wrapper function for
os_file_create_simple() which opens or creates a file.
@param[in]	key		Performance Schema Key
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	access_type	OS_FILE_READ_ONLY or OS_FILE_READ_WRITE
@param[in]	read_only	if true read only mode checks are enforced
@param[out]	success		true if succeeded
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return own: handle to the file, not defined if error, error number
        can be retrieved with os_file_get_last_error */
UNIV_INLINE
pfs_os_file_t pfs_os_file_create_simple_func(
    mysql_pfs_key_t key, const char *name, ulint create_mode, ulint access_type,
    bool read_only, bool *success, const char *src_file, uint src_line)
    MY_ATTRIBUTE((warn_unused_result));

/** NOTE! Please use the corresponding macro
os_file_create_simple_no_error_handling(), not directly this function!
A performance schema instrumented wrapper function for
os_file_create_simple_no_error_handling(). Add instrumentation to
monitor file creation/open.
@param[in]	key		Performance Schema Key
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	access_type	OS_FILE_READ_ONLY, OS_FILE_READ_WRITE, or
                                OS_FILE_READ_ALLOW_DELETE; the last option is
                                used by a backup program reading the file
@param[in]	read_only	if true read only mode checks are enforced
@param[out]	success		true if succeeded
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return own: handle to the file, not defined if error, error number
        can be retrieved with os_file_get_last_error */
UNIV_INLINE
pfs_os_file_t pfs_os_file_create_simple_no_error_handling_func(
    mysql_pfs_key_t key, const char *name, ulint create_mode, ulint access_type,
    bool read_only, bool *success, const char *src_file, uint src_line)
    MY_ATTRIBUTE((warn_unused_result));

/** NOTE! Please use the corresponding macro os_file_create(), not directly
this function!
A performance schema wrapper function for os_file_create().
Add instrumentation to monitor file creation/open.
@param[in]	key		Performance Schema Key
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	purpose		OS_FILE_AIO, if asynchronous, non-buffered I/O
                                is desired, OS_FILE_NORMAL, if any normal file;
                                NOTE that it also depends on type, os_aio_..
                                and srv_.. variables whether we really use
                                async I/O or unbuffered I/O: look in the
                                function source code for the exact rules
@param[in]	type		OS_DATA_FILE or OS_LOG_FILE
@param[in]	read_only	if true read only mode checks are enforced
@param[out]	success		true if succeeded
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return own: handle to the file, not defined if error, error number
        can be retrieved with os_file_get_last_error */
UNIV_INLINE
pfs_os_file_t pfs_os_file_create_func(mysql_pfs_key_t key, const char *name,
                                      ulint create_mode, ulint purpose,
                                      ulint type, bool read_only, bool *success,
                                      const char *src_file, uint src_line)
    MY_ATTRIBUTE((warn_unused_result));

/** NOTE! Please use the corresponding macro os_file_close(), not directly
this function!
A performance schema instrumented wrapper function for os_file_close().
@param[in]	file		handle to a file
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return true if success */
UNIV_INLINE
bool pfs_os_file_close_func(pfs_os_file_t file, const char *src_file,
                            uint src_line);

/** NOTE! Please use the corresponding macro os_file_read(), not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_read() which requests a synchronous read operation.
@param[in, out]	type		IO request context
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if request was successful */
UNIV_INLINE
dberr_t pfs_os_file_read_func(IORequest &type, pfs_os_file_t file, void *buf,
                              os_offset_t offset, ulint n, const char *src_file,
                              uint src_line);

/** NOTE! Please use the corresponding macro os_file_read_first_page(),
not directly this function!
This is the performance schema instrumented wrapper function for
os_file_read_first_page() which requests a synchronous read operation
of page 0 of IBD file
@param[in, out]	type		IO request context
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	n		number of bytes to read
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if request was successful */
UNIV_INLINE
dberr_t pfs_os_file_read_first_page_func(IORequest &type, pfs_os_file_t file,
                                         void *buf, ulint n,
                                         const char *src_file, uint src_line);

/** copy data from one file to another file. Data is read/written
at current file offset.
@param[in]	src		file handle to copy from
@param[in]	src_offset	offset to copy from
@param[in]	dest		file handle to copy to
@param[in]	dest_offset	offset to copy to
@param[in]	size		number of bytes to copy
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if successful */
UNIV_INLINE
dberr_t pfs_os_file_copy_func(pfs_os_file_t src, os_offset_t src_offset,
                              pfs_os_file_t dest, os_offset_t dest_offset,
                              uint size, const char *src_file, uint src_line);

/** NOTE! Please use the corresponding macro os_file_read_no_error_handling(),
not directly this function!
This is the performance schema instrumented wrapper function for
os_file_read_no_error_handling_func() which requests a synchronous
read operation.
@param[in, out]	type		IO request context
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@param[out]	o		number of bytes actually read
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if request was successful */
UNIV_INLINE
dberr_t pfs_os_file_read_no_error_handling_func(IORequest &type,
                                                pfs_os_file_t file, void *buf,
                                                os_offset_t offset, ulint n,
                                                ulint *o, const char *src_file,
                                                uint src_line);

/** NOTE! Please use the corresponding macro
os_file_read_no_error_handling_int_fd(), not directly this function!
This is the performance schema instrumented wrapper function for
os_file_read_no_error_handling_int_fd_func() which requests a
synchronous read operation on files with int type descriptors.
@param[in, out] type            IO request context
@param[in]      file            Open file handle
@param[out]     buf             buffer where to read
@param[in]      offset          file offset where to read
@param[in]      n               number of bytes to read
@param[out]     o               number of bytes actually read
@param[in]      src_file        file name where func invoked
@param[in]      src_line        line where the func invoked
@return DB_SUCCESS if request was successful */

UNIV_INLINE
dberr_t pfs_os_file_read_no_error_handling_int_fd_func(
    IORequest &type, int file, void *buf, os_offset_t offset, ulint n, ulint *o,
    const char *src_file, ulint src_line);

/** NOTE! Please use the corresponding macro os_aio(), not directly this
function!
Performance schema wrapper function of os_aio() which requests
an asynchronous I/O operation.
@param[in]	type		IO request context
@param[in]	mode		IO mode
@param[in]	name		Name of the file or path as NUL terminated
                                string
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@param[in]	read_only	if true read only mode checks are enforced
@param[in,out]	m1		Message for the AIO handler, (can be used to
                                identify a completed AIO operation); ignored
                                if mode is OS_AIO_SYNC
@param[in,out]	m2		message for the AIO handler (can be used to
                                identify a completed AIO operation); ignored
                                if mode is OS_AIO_SYNC
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if request was queued successfully, false if fail */
UNIV_INLINE
dberr_t pfs_os_aio_func(IORequest &type, AIO_mode mode, const char *name,
                        pfs_os_file_t file, void *buf, os_offset_t offset,
                        ulint n, bool read_only, fil_node_t *m1, void *m2,
                        const char *src_file, uint src_line);

/** NOTE! Please use the corresponding macro os_file_write(), not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_write() which requests a synchronous write operation.
@param[in, out]	type		IO request context
@param[in]	name		Name of the file or path as NUL terminated
                                string
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if request was successful */
UNIV_INLINE
dberr_t pfs_os_file_write_func(IORequest &type, const char *name,
                               pfs_os_file_t file, const void *buf,
                               os_offset_t offset, ulint n,
                               const char *src_file, uint src_line);

/** NOTE! Please use the corresponding macro os_file_write(), not
directly this function!
This is the performance schema instrumented wrapper function for
os_file_write() which requests a synchronous write operation
on files with int type descriptors.
@param[in, out] type            IO request context
@param[in]      name            Name of the file or path as NUL terminated
                                string
@param[in]      file            Open file handle
@param[out]     buf             buffer where to read
@param[in]      offset          file offset where to read
@param[in]      n		number of bytes to read
@param[in]      src_file        file name where func invoked
@param[in]      src_line        line where the func invoked
@return DB_SUCCESS if request was successful */
UNIV_INLINE
dberr_t pfs_os_file_write_int_fd_func(IORequest &type, const char *name,
                                      int file, const void *buf,
                                      os_offset_t offset, ulint n,
                                      const char *src_file, ulint src_line);

/** NOTE! Please use the corresponding macro os_file_flush(), not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_flush() which flushes the write buffers of a given file to the disk.
Flushes the write buffers of a given file to the disk.
@param[in]	file		Open file handle
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return true if success */
UNIV_INLINE
bool pfs_os_file_flush_func(pfs_os_file_t file, const char *src_file,
                            uint src_line);

/** NOTE! Please use the corresponding macro os_file_rename(), not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_rename()
@param[in]	key		Performance Schema Key
@param[in]	oldpath		old file path as a null-terminated string
@param[in]	newpath		new file path
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return true if success */
UNIV_INLINE
bool pfs_os_file_rename_func(mysql_pfs_key_t key, const char *oldpath,
                             const char *newpath, const char *src_file,
                             uint src_line);

/**
NOTE! Please use the corresponding macro os_file_delete(), not directly
this function!
This is the performance schema instrumented wrapper function for
os_file_delete()
@param[in]	key		Performance Schema Key
@param[in]	name		old file path as a null-terminated string
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return true if success */
UNIV_INLINE
bool pfs_os_file_delete_func(mysql_pfs_key_t key, const char *name,
                             const char *src_file, uint src_line);

/**
NOTE! Please use the corresponding macro os_file_delete_if_exists(), not
directly this function!
This is the performance schema instrumented wrapper function for
os_file_delete_if_exists()
@param[in]	key		Performance Schema Key
@param[in]	name		old file path as a null-terminated string
@param[in]	exist		indicate if file pre-exist
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return true if success */
UNIV_INLINE
bool pfs_os_file_delete_if_exists_func(mysql_pfs_key_t key, const char *name,
                                       bool *exist, const char *src_file,
                                       uint src_line);

#else /* UNIV_PFS_IO */

/* If UNIV_PFS_IO is not defined, these I/O APIs point
to original un-instrumented file I/O APIs */
#define os_file_create(key, name, create, purpose, type, read_only, success) \
  os_file_create_func(name, create, purpose, type, read_only, success)

#define os_file_create_simple(key, name, create_mode, access, read_only, \
                              success)                                   \
  os_file_create_simple_func(name, create_mode, access, read_only, success)

#define os_file_create_simple_no_error_handling(key, name, create_mode,     \
                                                access, read_only, success) \
  os_file_create_simple_no_error_handling_func(name, create_mode, access,   \
                                               read_only, success)

#define os_file_close_pfs(file) os_file_close_func(file)

#define os_aio(type, mode, name, file, buf, offset, n, read_only, message1, \
               message2)                                                    \
  os_aio_func(type, mode, name, file, buf, offset, n, read_only, message1,  \
              message2)

#define os_file_read_pfs(type, file, buf, offset, n) \
  os_file_read_func(type, file, buf, offset, n)

#define os_file_read_first_page_pfs(type, file, buf, n) \
  os_file_read_first_page_func(type, file, buf, n)

#define os_file_copy_pfs(src, src_offset, dest, dest_offset, size) \
  os_file_copy_func(src, src_offset, dest, dest_offset, size)

#define os_file_read_no_error_handling_pfs(type, file, buf, offset, n, o) \
  os_file_read_no_error_handling_func(type, file, buf, offset, n, o)

#define os_file_read_no_error_handling_int_fd(type, file, buf, offset, n, o) \
  os_file_read_no_error_handling_func(type, file, buf, offset, n, o)

#define os_file_write_pfs(type, name, file, buf, offset, n) \
  os_file_write_func(type, name, file, buf, offset, n)

#define os_file_write_int_fd(type, name, file, buf, offset, n) \
  os_file_write_func(type, name, file, buf, offset, n)

#define os_file_flush_pfs(file) os_file_flush_func(file)

#define os_file_rename(key, oldpath, newpath) \
  os_file_rename_func(oldpath, newpath)

#define os_file_delete(key, name) os_file_delete_func(name)

#define os_file_delete_if_exists(key, name, exist) \
  os_file_delete_if_exists_func(name, exist)

#endif /* UNIV_PFS_IO */

#ifdef UNIV_PFS_IO
#define os_file_close(file) os_file_close_pfs(file)
#else
#define os_file_close(file) os_file_close_pfs((file).m_file)
#endif

#ifdef UNIV_PFS_IO
#define os_file_read(type, file, buf, offset, n) \
  os_file_read_pfs(type, file, buf, offset, n)
#else
#define os_file_read(type, file, buf, offset, n) \
  os_file_read_pfs(type, file.m_file, buf, offset, n)
#endif

#ifdef UNIV_PFS_IO
#define os_file_read_first_page(type, file, buf, n) \
  os_file_read_first_page_pfs(type, file, buf, n)
#else
#define os_file_read_first_page(type, file, buf, n) \
  os_file_read_first_page_pfs(type, file.m_file, buf, n)
#endif

#ifdef UNIV_PFS_IO
#define os_file_flush(file) os_file_flush_pfs(file)
#else
#define os_file_flush(file) os_file_flush_pfs(file.m_file)
#endif

#ifdef UNIV_PFS_IO
#define os_file_write(type, name, file, buf, offset, n) \
  os_file_write_pfs(type, name, file, buf, offset, n)
#else
#define os_file_write(type, name, file, buf, offset, n) \
  os_file_write_pfs(type, name, file.m_file, buf, offset, n)
#endif

#ifdef UNIV_PFS_IO
#define os_file_copy(src, src_offset, dest, dest_offset, size) \
  os_file_copy_pfs(src, src_offset, dest, dest_offset, size)
#else
#define os_file_copy(src, src_offset, dest, dest_offset, size) \
  os_file_copy_pfs(src.m_file, src_offset, dest.m_file, dest_offset, size)
#endif

#ifdef UNIV_PFS_IO
#define os_file_read_no_error_handling(type, file, buf, offset, n, o) \
  os_file_read_no_error_handling_pfs(type, file, buf, offset, n, o)
#else
#define os_file_read_no_error_handling(type, file, buf, offset, n, o) \
  os_file_read_no_error_handling_pfs(type, file.m_file, buf, offset, n, o)
#endif

#ifdef UNIV_HOTBACKUP
/** Closes a file handle.
@param[in] file		handle to a file
@return true if success */
bool os_file_close_no_error_handling(os_file_t file);
#endif /* UNIV_HOTBACKUP */


/** Write the specified number of zeros to a file from specific offset.
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	file		handle to a file
@param[in]	offset		file offset
@param[in]	size		file size
@param[in]	read_only	Enable read-only checks if true
@param[in]	flush		Flush file content to disk
@return true if success */
bool os_file_set_size(const char *name, pfs_os_file_t file, os_offset_t offset,
                      os_offset_t size, bool read_only, bool flush)
    MY_ATTRIBUTE((warn_unused_result));

/** Truncates a file at its current position.
@param[in,out]	file	file to be truncated
@return true if success */
bool os_file_set_eof(FILE *file); /*!< in: file to be truncated */

/** Truncates a file to a specified size in bytes. Do nothing if the size
preserved is smaller or equal than current size of file.
@param[in]	pathname	file path
@param[in]	file		file to be truncated
@param[in]	size		size preserved in bytes
@return true if success */
bool os_file_truncate(const char *pathname, pfs_os_file_t file,
                      os_offset_t size);

/** Set read/write position of a file handle to specific offset.
@param[in]	pathname	file path
@param[in]	file		file handle
@param[in]	offset		read/write offset
@return true if success */
bool os_file_seek(const char *pathname, os_file_t file, os_offset_t offset);


/** Retrieves the last error number if an error occurs in a file io function.
The number should be retrieved before any other OS calls (because they may
overwrite the error number). If the number is not known to this program,
the OS error number + 100 is returned.
@param[in]	report_all_errors	true if we want an error message printed
                                        for all errors
@return error number, or OS error number + 100 */
ulint os_file_get_last_error(bool report_all_errors);

/** NOTE! Use the corresponding macro os_file_read(), not directly this
function!
Requests a synchronous read operation.
@param[in]	type		IO request context
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@return DB_SUCCESS if request was successful */
dberr_t os_file_read_func(IORequest &type, os_file_t file, void *buf,
                          os_offset_t offset, ulint n)
    MY_ATTRIBUTE((warn_unused_result));

/** NOTE! Use the corresponding macro os_file_read_first_page(),
not directly this function!
Requests a synchronous read operation of page 0 of IBD file
@param[in]	type		IO request context
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	n		number of bytes to read
@return DB_SUCCESS if request was successful */
dberr_t os_file_read_first_page_func(IORequest &type, os_file_t file, void *buf,
                                     ulint n)
    MY_ATTRIBUTE((warn_unused_result));

/** copy data from one file to another file. Data is read/written
at current file offset.
@param[in]	src_file	file handle to copy from
@param[in]	src_offset	offset to copy from
@param[in]	dest_file	file handle to copy to
@param[in]	dest_offset	offset to copy to
@param[in]	size		number of bytes to copy
@return DB_SUCCESS if successful */
dberr_t os_file_copy_func(os_file_t src_file, os_offset_t src_offset,
                          os_file_t dest_file, os_offset_t dest_offset,
                          uint size) MY_ATTRIBUTE((warn_unused_result));

/** Rewind file to its start, read at most size - 1 bytes from it to str, and
NUL-terminate str. All errors are silently ignored. This function is
mostly meant to be used with temporary files.
@param[in,out]	file		file to read from
@param[in,out]	str		buffer where to read
@param[in]	size		size of buffer */
void os_file_read_string(FILE *file, char *str, ulint size);

/** NOTE! Use the corresponding macro os_file_read_no_error_handling(),
not directly this function!
Requests a synchronous positioned read operation. This function does not do
any error handling. In case of error it returns FALSE.
@param[in]	type		IO request context
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@param[out]	o		number of bytes actually read
@return DB_SUCCESS or error code */
dberr_t os_file_read_no_error_handling_func(IORequest &type, os_file_t file,
                                            void *buf, os_offset_t offset,
                                            ulint n, ulint *o)
    MY_ATTRIBUTE((warn_unused_result));

/** NOTE! Use the corresponding macro os_file_write(), not directly this
function!
Requests a synchronous write operation.
@param[in,out]	type		IO request context
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@return DB_SUCCESS if request was successful */
dberr_t os_file_write_func(IORequest &type, const char *name, os_file_t file,
                           const void *buf, os_offset_t offset, ulint n)
    MY_ATTRIBUTE((warn_unused_result));

/** Check the existence and type of the given file.
@param[in]	path		pathname of the file
@param[out]	exists		true if file exists
@param[out]	type		type of the file (if it exists)
@return true if call succeeded */
bool os_file_status(const char *path, bool *exists, os_file_type_t *type);

/** Create all missing subdirectories along the given path.
@return DB_SUCCESS if OK, otherwise error code. */
dberr_t os_file_create_subdirs_if_needed(const char *path);

#ifdef UNIV_ENABLE_UNIT_TEST_GET_PARENT_DIR
/* Test the function os_file_get_parent_dir. */
void unit_test_os_file_get_parent_dir();
#endif /* UNIV_ENABLE_UNIT_TEST_GET_PARENT_DIR */



/** Initializes the asynchronous io system. Creates one array each for ibuf
and log i/o. Also creates one array each for read and write where each
array is divided logically into n_read_segs and n_write_segs
respectively. The caller must create an i/o handler thread for each
segment in these arrays. This function also creates the sync array.
No i/o handler thread needs to be created for that
@param[in]	n_readers	number of reader threads
@param[in]	n_writers	number of writer threads
@param[in]	n_slots_sync	number of slots in the sync aio array */

bool os_aio_init(ulint n_readers, ulint n_writers, ulint n_slots_sync);

/**
Frees the asynchronous io system. */
void os_aio_free();

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
                                if mode is OS_AIO_SYNC
@param[in,out]	m2		message for the AIO handler (can be used to
                                identify a completed AIO operation); ignored
                                if mode is OS_AIO_SYNC
@return DB_SUCCESS or error code */
dberr_t os_aio_func(IORequest &type, AIO_mode aio_mode, const char *name,
                    pfs_os_file_t file, void *buf, os_offset_t offset, ulint n,
                    bool read_only, fil_node_t *m1, void *m2);

/** Wakes up all async i/o threads so that they know to exit themselves in
shutdown. */
void os_aio_wake_all_threads_at_shutdown();

/** Waits until there are no pending writes in os_aio_write_array. There can
be other, synchronous, pending writes. */
void os_aio_wait_until_no_pending_writes();

/** Wakes up simulated aio i/o-handler threads if they have something to do. */
void os_aio_simulated_wake_handler_threads();

/** This function can be called if one wants to post a batch of reads and
prefers an i/o-handler thread to handle them all at once later. You must
call os_aio_simulated_wake_handler_threads later to ensure the threads
are not left sleeping! */
void os_aio_simulated_put_read_threads_to_sleep();

/** This is the generic AIO handler interface function.
Waits for an aio operation to complete. This function is used to wait the
for completed requests. The AIO array of pending requests is divided
into segments. The thread specifies which segment or slot it wants to wait
for. NOTE: this function will also take care of freeing the aio slot,
therefore no other thread is allowed to do the freeing!
@param[in]	segment		the number of the segment in the aio arrays to
                                wait for; segment 0 is the ibuf I/O thread,
                                segment 1 the log I/O thread, then follow the
                                non-ibuf read threads, and as the last are the
                                non-ibuf write threads; if this is
                                ULINT_UNDEFINED, then it means that sync AIO
                                is used, and this parameter is ignored
@param[out]	m1		the messages passed with the AIO request;
                                note that also in the case where the AIO
                                operation failed, these output parameters
                                are valid and can be used to restart the
                                operation, for example
@param[out]	m2		callback message
@param[out]	request		OS_FILE_WRITE or ..._READ
@return DB_SUCCESS or error code */
dberr_t os_aio_handler(ulint segment, fil_node_t **m1, void **m2,
                       IORequest *request);

/** Prints info of the aio arrays.
@param[in,out]	file		file where to print */
void os_aio_print(FILE *file);

/** Refreshes the statistics used to print per-second averages. */
void os_aio_refresh_stats();

/** Checks that all slots in the system have been freed, that is, there are
no pending io operations. */
bool os_aio_all_slots_free();

#ifdef UNIV_DEBUG

/** Prints all pending IO
@param[in]	file	file where to print */
void os_aio_print_pending_io(FILE *file);

#endif /* UNIV_DEBUG */

/** This function returns information about the specified file
@param[in]	path		pathname of the file
@param[in]	stat_info	information of a file in a directory
@param[in]	check_rw_perm	for testing whether the file can be opened
                                in RW mode
@param[in]	read_only	if true read only mode checks are enforced
@return DB_SUCCESS if all OK */
dberr_t os_file_get_status(const char *path, os_file_stat_t *stat_info,
                           bool check_rw_perm, bool read_only);

#ifndef UNIV_HOTBACKUP

/** return any of the tmpdir path */
char *innobase_mysql_tmpdir();
/** Creates a temporary file in the location specified by the parameter
path. If the path is NULL then it will be created on --tmpdir location.
This function is defined in ha_innodb.cc.
@param[in]	path	location for creating temporary file
@return temporary file descriptor, or < 0 on error */
int innobase_mysql_tmpfile(const char *path);
#endif /* !UNIV_HOTBACKUP */

/** If it is a compressed page return the compressed page data + footer size
@param[in]	buf		Buffer to check, must include header + 10 bytes
@return ULINT_UNDEFINED if the page is not a compressed page or length
        of the compressed data (including footer) if it is a compressed page */
ulint os_file_compressed_page_size(const byte *buf);

/** If it is a compressed page return the original page data + footer size
@param[in]	buf		Buffer to check, must include header + 10 bytes
@return ULINT_UNDEFINED if the page is not a compressed page or length
        of the original data + footer if it is a compressed page */
ulint os_file_original_page_size(const byte *buf);

/** Set the file create umask
@param[in]	umask		The umask to use for file creation. */
void os_file_set_umask(ulint umask);

/** Free storage space associated with a section of the file.
@param[in]	fh		Open file handle
@param[in]	off		Starting offset (SEEK_SET)
@param[in]	len		Size of the hole
@return DB_SUCCESS or error code */
dberr_t os_file_punch_hole(os_file_t fh, os_offset_t off, os_offset_t len)
    MY_ATTRIBUTE((warn_unused_result));

/** Check if the file system supports sparse files.

Warning: On POSIX systems we try and punch a hole from offset 0 to
the system configured page size. This should only be called on an empty
file.

Note: On Windows we use the name and on Unices we use the file handle.

@param[in]	path	File name
@param[in]	fh	File handle for the file - if opened
@return true if the file system supports sparse files */
bool os_is_sparse_file_supported(const char *path, pfs_os_file_t fh)
    MY_ATTRIBUTE((warn_unused_result));

/** Decompress the page data contents. Page type must be FIL_PAGE_COMPRESSED, if
not then the source contents are left unchanged and DB_SUCCESS is returned.
@param[in]	dblwr_recover	true of double write recovery in progress
@param[in,out]	src		Data read from disk, decompressed data will be
                                copied to this page
@param[in,out]	dst		Scratch area to use for decompression
@param[in]	dst_len		Size of the scratch area in bytes
@return DB_SUCCESS or error code */
dberr_t os_file_decompress_page(bool dblwr_recover, byte *src, byte *dst,
                                ulint dst_len)
    MY_ATTRIBUTE((warn_unused_result));

/** Determine if O_DIRECT is supported.
@retval	true	if O_DIRECT is supported.
@retval	false	if O_DIRECT is not supported. */
bool os_is_o_direct_supported() MY_ATTRIBUTE((warn_unused_result));

/** Class to scan the directory heirarch using a depth first scan. */
class Dir_Walker {
 public:
  using Path = std::string;

  /** Check if the path is a directory. The file/directory must exist.
  @param[in]	path		The path to check
  @return true if it is a directory */
  static bool is_directory(const Path &path);

  /** Depth first traversal of the directory starting from basedir
  @param[in]	basedir		Start scanning from this directory
  @param[in]    recursive       True if scan should be recursive
  @param[in]	f		Function to call for each entry */
  template <typename F>
  static void walk(const Path &basedir, bool recursive, F &&f) {
#ifdef _WIN32
    walk_win32(basedir, recursive,
               [&](const Path &path, size_t depth) { f(path); });
#else
    walk_posix(basedir, recursive,
               [&](const Path &path, size_t depth) { f(path); });
#endif /* _WIN32 */
  }

 private:
  /** Directory names for the depth first directory scan. */
  struct Entry {
    /** Constructor
    @param[in]	path		Directory to traverse
    @param[in]	depth		Relative depth to the base
                                    directory in walk() */
    Entry(const Path &path, size_t depth) : m_path(path), m_depth(depth) {}

    /** Path to the directory */
    Path m_path;

    /** Relative depth of m_path */
    size_t m_depth;
  };

  using Function = std::function<void(const Path &, size_t)>;

  /** Depth first traversal of the directory starting from basedir
  @param[in]	basedir		Start scanning from this directory
  @param[in]    recursive       True if scan should be recursive
  @param[in]	f		Function to call for each entry */
#ifdef _WIN32
  static void walk_win32(const Path &basedir, bool recursive, Function &&f);
#else
  static void walk_posix(const Path &basedir, bool recursive, Function &&f);
#endif /* _WIN32 */
};

#include "os0file.ic"

#endif /* os0file_h */
