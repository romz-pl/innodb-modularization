/*****************************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

*****************************************************************************/

/** @file fil/fil0fil.cc
The tablespace memory cache */

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include <innodb/trx_purge/is_active.h>
#include <innodb/trx_purge/spaces.h>
#include <innodb/trx_purge/id2num.h>
#include <innodb/trx_purge/is_active_truncate_log_present.h>
#include <innodb/trx_trx/TrxInInnoDB.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/trx_trx/trx_start_if_not_started_xa.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/clone/clone_mark_active.h>
#include <innodb/clone/clone_mark_abort.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/io/os_file_create_directory.h>
#include <innodb/io/Dir_Walker.h>
#include <innodb/io/access_type.h>
#include <innodb/io/os_file_get_size.h>
#include <innodb/io/os_file_punch_hole.h>
#include <innodb/io/os_is_sparse_file_supported.h>
#include <innodb/io/os_file_create_subdirs_if_needed.h>
#include <innodb/io/os_file_truncate.h>
#include <innodb/io/os_file_create_simple_no_error_handling.h>
#include <innodb/io/os_file_create.h>
#include <innodb/io/os_file_read.h>
#include <innodb/io/os_file_flush.h>
#include <innodb/time/ut_time.h>
#include <innodb/logger/info.h>
#include <innodb/formatting/hex.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_wait_low.h>
#include <innodb/tablespace/fil_load_status.h>
#include <innodb/tablespace/fil_operation_t.h>
#include <innodb/tablespace/fil_load_status.h>
#include <innodb/tablespace/Char_Ptr_Compare.h>
#include <innodb/tablespace/Tablespaces.h>
#include <innodb/tablespace/Dirs.h>
#include <innodb/tablespace/Space_id_set.h>
#include <innodb/tablespace/Scanned_files.h>
#include <innodb/tablespace/Char_Ptr_Hash.h>
#include <innodb/tablespace/Tablespace_files.h>
#include <innodb/tablespace/Tablespace_dirs.h>
#include <innodb/buf_block/buf_block_set_file_page.h>
#include <innodb/buf_block/buf_block_set_state.h>
#include <innodb/buf_block/buf_block_get_state.h>
#include <innodb/io/os_has_said_disk_full.h>
#include <innodb/io/IORequestRead.h>
#include <innodb/io/IORequestWrite.h>
#include <innodb/io/pfs.h>
#include <innodb/ioasync/os_aio_simulated_wake_handler_threads.h>
#include <innodb/ioasync/os_file_set_size.h>
#include <innodb/ioasync/os_aio_func.h>
#include <innodb/ioasync/os_aio.h>
#include <innodb/ioasync/os_aio_handler.h>
#include <innodb/tablespace/os_file_read_first_page.h>
#include <innodb/tablespace/fil_buffering_disabled.h>
#include <innodb/tablespace/Datafile.h>
#include <innodb/tablespace/srv_tmp_space.h>
#include <innodb/tablespace/srv_sys_space.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/tablespace/fil_write_zeros.h>
#include <innodb/io/os_file_delete_if_exists.h>
#include <innodb/io/os_file_delete.h>
#include <innodb/io/os_file_rename.h>
#include <innodb/io/os_file_write.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_get_last_error.h>
#include <innodb/page/page_zip_set_size.h>
#include <innodb/page/page_zip_des_init.h>
#include <innodb/log_types/recv_recovery_is_on.h>

#include "btr0btr.h"
#include "buf0buf.h"
#include "buf0flu.h"
#include "dict0boot.h"
#include "dict0dd.h"
#include "dict0dict.h"
#include "fsp0fsp.h"
#include "buf0dblwr.h"

#include "ha_prototypes.h"

#include "log0recv.h"
#include <innodb/machine/data.h>

#include "mtr0log.h"
#include "my_dbug.h"
#include "my_inttypes.h"

#include "srv0start.h"
#include <innodb/univ/call_destructor.h>
#include <innodb/enum/to_int.h>

#ifndef UNIV_HOTBACKUP
#include "buf0lru.h"
#include "ibuf0ibuf.h"

#include "row0mysql.h"
#include "sql_backup_lock.h"
#include "sql_class.h"


#else /* !UNIV_HOTBACKUP */
#include <cstring>
#include "srv0srv.h"
#endif /* !UNIV_HOTBACKUP */

#include <innodb/thread/os_thread_create.h>

#include "current_thd.h"
#include "ha_prototypes.h"

#include <array>
#include <fstream>
#include <functional>
#include <list>
#include <mutex>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <innodb/tablespace/Fil_shard.h>
#include <innodb/tablespace/Fil_system.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fil_n_file_opened.h>
#include <innodb/io/MySQL_datadir_path.h>
#include <innodb/tablespace/fil_n_log_flushes.h>
#include <innodb/tablespace/fil_n_pending_log_flushes.h>
#include <innodb/tablespace/fil_n_pending_tablespace_flushes.h>
#include <innodb/tablespace/fil_addr_null.h>
#include <innodb/tablespace/fil_system.h>






#ifdef UNIV_HOTBACKUP
/** Directories in which remote general tablespaces have been found in the
target directory during apply log operation */
Dir_set rem_gen_ts_dirs;

/** true in case the apply-log operation is being performed
in the data directory */
bool replay_in_datadir = false;

/* Re-define mutex macros to use the Mutex class defined by the MEB
source. MEB calls the routines in "fil0fil.cc" in parallel and,
therefore, the mutex protecting the critical sections of the tablespace
memory cache must be included also in the MEB compilation of this
module. */
#undef mutex_create
#undef mutex_free
#undef mutex_enter
#undef mutex_exit
#undef mutex_own
#undef mutex_validate

#define mutex_create(I, M) new (M) meb::Mutex()
#define mutex_free(M) delete (M)
#define mutex_enter(M) (M)->lock()
#define mutex_exit(M) (M)->unlock()
#define mutex_own(M) 1
#define mutex_validate(M) 1

/** Process a MLOG_FILE_CREATE redo record.
@param[in]	page_id		Page id of the redo log record
@param[in]	flags		Tablespace flags
@param[in]	name		Tablespace filename */
static void meb_tablespace_redo_create(const page_id_t &page_id, uint32_t flags,
                                       const char *name);

/** Process a MLOG_FILE_RENAME redo record.
@param[in]	page_id		Page id of the redo log record
@param[in]	from_name	Tablespace from filename
@param[in]	to_name		Tablespace to filename */
static void meb_tablespace_redo_rename(const page_id_t &page_id,
                                       const char *from_name,
                                       const char *to_name);

/** Process a MLOG_FILE_DELETE redo record.
@param[in]	page_id		Page id of the redo log record
@param[in]	name		Tablespace filename */
static void meb_tablespace_redo_delete(const page_id_t &page_id,
                                       const char *name);

#endif /* UNIV_HOTBACKUP */

static void fil_op_write_log(mlog_id_t type, space_id_t space_id,
                             const char *path, const char *new_path,
                             uint32_t flags, mtr_t *mtr);

/** Create a tablespace (an IBD or IBT) file
@param[in]	space_id	Tablespace ID
@param[in]	name		Tablespace name in dbname/tablename format.
                                For general tablespaces, the 'dbname/' part
                                may be missing.
@param[in]	path		Path and filename of the datafile to create.
@param[in]	flags		Tablespace flags
@param[in]	size		Initial size of the tablespace file in pages,
                                must be >= FIL_IBD_FILE_INITIAL_SIZE
@param[in]	type		FIL_TYPE_TABLESPACE or FIL_TYPE_TEMPORARY
@return DB_SUCCESS or error code */
dberr_t fil_create_tablespace(space_id_t space_id, const char *name,
                                     const char *path, uint32_t flags,
                                     page_no_t size, fil_type_t type) {
  pfs_os_file_t file;
  dberr_t err;
  byte *buf2;
  byte *page;
  bool success;
  bool has_shared_space = FSP_FLAGS_GET_SHARED(flags);
  fil_space_t *space = nullptr;

  ut_ad(!fsp_is_system_tablespace(space_id));
  ut_ad(!fsp_is_global_temporary(space_id));
  ut_a(fsp_flags_is_valid(flags));
  ut_a(type == FIL_TYPE_TEMPORARY || type == FIL_TYPE_TABLESPACE);

  const page_size_t page_size(flags);

  /* Create the subdirectories in the path, if they are
  not there already. */
  if (!has_shared_space) {
    err = os_file_create_subdirs_if_needed(path);

    if (err != DB_SUCCESS) {
      return (err);
    }
  }

  file = os_file_create(
      type == FIL_TYPE_TEMPORARY ? innodb_temp_file_key : innodb_data_file_key,
      path, OS_FILE_CREATE | OS_FILE_ON_ERROR_NO_EXIT, OS_FILE_NORMAL,
      OS_DATA_FILE, srv_read_only_mode && (type != FIL_TYPE_TEMPORARY),
      &success);

  if (!success) {
    /* The following call will print an error message */
    ulint error = os_file_get_last_error(true);

    ib::error(ER_IB_MSG_301, path);

    if (error == OS_FILE_ALREADY_EXISTS) {
      ib::error(ER_IB_MSG_302, path, path);

      return (DB_TABLESPACE_EXISTS);
    }

    if (error == OS_FILE_DISK_FULL) {
      return (DB_OUT_OF_DISK_SPACE);
    }

    return (DB_ERROR);
  }

  bool atomic_write;

#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)
  if (fil_fusionio_enable_atomic_write(file)) {
    int ret = posix_fallocate(file.m_file, 0, size * page_size.physical());

    if (ret != 0) {
      ib::error(ER_IB_MSG_303, path, ulonglong{size * page_size.physical()},
                ret, REFMAN);
      success = false;
    } else {
      success = true;
    }

    atomic_write = true;
  } else {
    atomic_write = false;

    success = os_file_set_size(path, file, 0, size * page_size.physical(),
                               srv_read_only_mode, true);
  }
#else
  atomic_write = false;

  success = os_file_set_size(path, file, 0, size * page_size.physical(),
                             srv_read_only_mode, true);

#endif /* !NO_FALLOCATE && UNIV_LINUX */

  if (!success) {
    os_file_close(file);
    os_file_delete(innodb_data_file_key, path);
    return (DB_OUT_OF_DISK_SPACE);
  }

  /* Note: We are actually punching a hole, previous contents will
  be lost after this call, if it succeeds. In this case the file
  should be full of NULs. */

  bool punch_hole = os_is_sparse_file_supported(path, file);

  if (punch_hole) {
    dberr_t punch_err;

    punch_err = os_file_punch_hole(file.m_file, 0, size * page_size.physical());

    if (punch_err != DB_SUCCESS) {
      punch_hole = false;
    }
  }

  /* We have to write the space id to the file immediately and flush the
  file to disk. This is because in crash recovery we must be aware what
  tablespaces exist and what are their space id's, so that we can apply
  the log records to the right file. It may take quite a while until
  buffer pool flush algorithms write anything to the file and flush it to
  disk. If we would not write here anything, the file would be filled
  with zeros from the call of os_file_set_size(), until a buffer pool
  flush would write to it. */

  buf2 = static_cast<byte *>(ut_malloc_nokey(3 * page_size.logical()));

  /* Align the memory for file i/o if we might have O_DIRECT set */
  page = static_cast<byte *>(ut_align(buf2, page_size.logical()));

  memset(page, '\0', page_size.logical());

  /* Add the UNIV_PAGE_SIZE to the table flags and write them to the
  tablespace header. */
  flags = fsp_flags_set_page_size(flags, page_size);
  fsp_header_init_fields(page, space_id, flags);
  mach_write_to_4(page + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID, space_id);

  mach_write_to_4(page + FIL_PAGE_SRV_VERSION, DD_SPACE_CURRENT_SRV_VERSION);
  mach_write_to_4(page + FIL_PAGE_SPACE_VERSION,
                  DD_SPACE_CURRENT_SPACE_VERSION);

  IORequest request(IORequest::WRITE);

  if (!page_size.is_compressed()) {
    buf_flush_init_for_writing(nullptr, page, nullptr, 0,
                               fsp_is_checksum_disabled(space_id),
                               true /* skip_lsn_check */);

    err = os_file_write(request, path, file, page, 0, page_size.physical());

    ut_ad(err != DB_IO_NO_PUNCH_HOLE);

  } else {
    page_zip_des_t page_zip;

    page_zip_set_size(&page_zip, page_size.physical());
    page_zip.data = page + page_size.logical();
#ifdef UNIV_DEBUG
    page_zip.m_start =
#endif /* UNIV_DEBUG */
        page_zip.m_end = page_zip.m_nonempty = page_zip.n_blobs = 0;

    buf_flush_init_for_writing(nullptr, page, &page_zip, 0,
                               fsp_is_checksum_disabled(space_id),
                               true /* skip_lsn_check */);

    err = os_file_write(request, path, file, page_zip.data, 0,
                        page_size.physical());

    ut_a(err != DB_IO_NO_PUNCH_HOLE);

    punch_hole = false;
  }

  ut_free(buf2);

  if (err != DB_SUCCESS) {
    ib::error(ER_IB_MSG_304, path);

    os_file_close(file);
    os_file_delete(innodb_data_file_key, path);

    return (DB_ERROR);
  }

  success = os_file_flush(file);

  if (!success) {
    ib::error(ER_IB_MSG_305, path);

    os_file_close(file);
    os_file_delete(innodb_data_file_key, path);
    return (DB_ERROR);
  }

  space = fil_space_create(name, space_id, flags, type);

  if (space == nullptr) {
    os_file_close(file);
    os_file_delete(innodb_data_file_key, path);
    return (DB_ERROR);
  }

  DEBUG_SYNC_C("fil_ibd_created_space");

  auto shard = fil_system->shard_by_id(space_id);

  fil_node_t *file_node =
      shard->create_node(path, size, space, false, punch_hole, atomic_write);

  err = (file_node == nullptr) ? DB_ERROR : DB_SUCCESS;

#ifndef UNIV_HOTBACKUP
  /* Temporary tablespace creation need not be redo logged */
  if (err == DB_SUCCESS && type != FIL_TYPE_TEMPORARY) {
    const auto &file = space->files.front();

    mtr_t mtr;

    mtr_start(&mtr);

    fil_op_write_log(MLOG_FILE_CREATE, space_id, file.name, nullptr,
                     space->flags, &mtr);

    mtr_commit(&mtr);

    DBUG_EXECUTE_IF("fil_ibd_create_log", log_make_latest_checkpoint(););
  }

#endif /* !UNIV_HOTBACKUP */

  /* For encryption tablespace, initial encryption information. */
  if (space != nullptr && FSP_FLAGS_GET_ENCRYPTION(space->flags)) {
    err = fil_set_encryption(space->id, Encryption::AES, nullptr, nullptr);

    ut_ad(err == DB_SUCCESS);
  }

  os_file_close(file);
  if (err != DB_SUCCESS) {
    os_file_delete(innodb_data_file_key, path);
  }

  return (err);
}




#ifdef UNIV_HOTBACKUP
static ulint srv_data_read;
static ulint srv_data_written;
#endif /* UNIV_HOTBACKUP */

/** Replay a file rename operation if possible.
@param[in]	page_id		Space ID and first page number in the file
@param[in]	old_name	old file name
@param[in]	new_name	new file name
@return	whether the operation was successfully applied (the name did not exist,
or new_name did not exist and name was successfully renamed to new_name)  */
static bool fil_op_replay_rename(const page_id_t &page_id,
                                 const std::string &old_name,
                                 const std::string &new_name)
    MY_ATTRIBUTE((warn_unused_result));





















#ifndef UNIV_HOTBACKUP
/** Write a log record about an operation on a tablespace file.
@param[in]	type		MLOG_FILE_OPEN or MLOG_FILE_DELETE
                                or MLOG_FILE_CREATE or MLOG_FILE_RENAME
@param[in]	space_id	tablespace identifier
@param[in]	path		file path
@param[in]	new_path	if type is MLOG_FILE_RENAME, the new name
@param[in]	flags		if type is MLOG_FILE_CREATE, the space flags
@param[in,out]	mtr		mini-transaction */
static void fil_op_write_log(mlog_id_t type, space_id_t space_id,
                             const char *path, const char *new_path,
                             uint32_t flags, mtr_t *mtr) {
  ut_ad(space_id != TRX_SYS_SPACE);

  byte *log_ptr;

  log_ptr = mlog_open(mtr, 11 + 4 + 2 + 1);

  if (log_ptr == nullptr) {
    /* Logging in mtr is switched off during crash recovery:
    in that case mlog_open returns nullptr */
    return;
  }

  log_ptr = mlog_write_initial_log_record_low(type, space_id, 0, log_ptr, mtr);

  if (type == MLOG_FILE_CREATE) {
    mach_write_to_4(log_ptr, flags);
    log_ptr += 4;
  }

  /* Let us store the strings as null-terminated for easier readability
  and handling */

  ulint len = strlen(path) + 1;

  mach_write_to_2(log_ptr, len);
  log_ptr += 2;

  mlog_close(mtr, log_ptr);

  mlog_catenate_string(mtr, reinterpret_cast<const byte *>(path), len);

  switch (type) {
    case MLOG_FILE_RENAME:

      ut_ad(strchr(new_path, Fil_path::OS_SEPARATOR) != nullptr);

      len = strlen(new_path) + 1;

      log_ptr = mlog_open(mtr, 2 + len);

      mach_write_to_2(log_ptr, len);

      log_ptr += 2;

      mlog_close(mtr, log_ptr);

      mlog_catenate_string(mtr, reinterpret_cast<const byte *>(new_path), len);
      break;
    case MLOG_FILE_DELETE:
    case MLOG_FILE_CREATE:
      break;
    default:
      ut_ad(0);
  }
}



#endif /* !UNIV_HOTBACKUP */

/** Deletes an IBD tablespace, either general or single-table.
The tablespace must be cached in the memory cache. This will delete the
datafile, fil_space_t & fil_node_t entries from the file_system_t cache.
@param[in]	space_id	Tablespace ID
@param[in]	buf_remove	Specify the action to take on the pages
                                for this table in the buffer pool.
@return DB_SUCCESS, DB_TABLESPCE_NOT_FOUND or DB_IO_ERROR */
dberr_t Fil_shard::space_delete(space_id_t space_id, buf_remove_t buf_remove) {
  char *path = nullptr;
  fil_space_t *space = nullptr;

  ut_ad(!fsp_is_system_tablespace(space_id));
  ut_ad(!fsp_is_global_temporary(space_id));

  dberr_t err = space_check_pending_operations(space_id, space, &path);

  if (err != DB_SUCCESS) {
    ut_a(err == DB_TABLESPACE_NOT_FOUND);

    ib::error(ER_IB_MSG_290, ulong{space_id});

    return (err);
  }

  ut_a(path != nullptr);
  ut_a(space != nullptr);

#ifndef UNIV_HOTBACKUP
  /* IMPORTANT: Because we have set space::stop_new_ops there
  can't be any new ibuf merges, reads or flushes. We are here
  because file::n_pending was zero above. However, it is still
  possible to have pending read and write requests:

  A read request can happen because the reader thread has
  gone through the ::stop_new_ops check in buf_page_init_for_read()
  before the flag was set and has not yet incremented ::n_pending
  when we checked it above.

  A write request can be issued any time because we don't check
  the ::stop_new_ops flag when queueing a block for write.

  We deal with pending write requests in the following function
  where we'd minimally evict all dirty pages belonging to this
  space from the flush_list. Note that if a block is IO-fixed
  we'll wait for IO to complete.

  To deal with potential read requests, we will check the
  ::stop_new_ops flag in fil_io(). */

  buf_LRU_flush_or_remove_pages(space_id, buf_remove, 0);

#endif /* !UNIV_HOTBACKUP */

  /* If it is a delete then also delete any generated files, otherwise
  when we drop the database the remove directory will fail. */
  if (space->purpose != FIL_TYPE_TEMPORARY) {
#ifdef UNIV_HOTBACKUP
  /* When replaying the operation in MySQL Enterprise
  Backup, we do not try to write any log record. */
#else /* UNIV_HOTBACKUP */
    /* Before deleting the file, write a log record about it, so that
    InnoDB crash recovery will expect the file to be gone.  Skip this
    for undo tablespaces since they use the trunc log file.  */
    if (!fsp_is_undo_tablespace(space_id)) {
      mtr_t mtr;
      mtr_start(&mtr);
      fil_op_write_log(MLOG_FILE_DELETE, space_id, path, nullptr, 0, &mtr);
      mtr_commit(&mtr);

      /* Even if we got killed shortly after deleting the
      tablespace file, the record must have already been
      written to the redo log. */
      log_write_up_to(*log_sys, mtr.commit_lsn(), true);
    }

#endif /* UNIV_HOTBACKUP */

    char *cfg_name = Fil_path::make_cfg(path);

    if (cfg_name != nullptr) {
      os_file_delete_if_exists(innodb_data_file_key, cfg_name, nullptr);

      ut_free(cfg_name);
    }

    char *cfp_name = Fil_path::make_cfp(path);

    if (cfp_name != nullptr) {
      os_file_delete_if_exists(innodb_data_file_key, cfp_name, nullptr);

      ut_free(cfp_name);
    }
  }

  /* Must set back to active before returning from function. */
  clone_mark_abort(true);

  mutex_acquire();

  /* Double check the sanity of pending ops after reacquiring
  the fil_system::mutex. */
  if (const fil_space_t *s = get_space_by_id(space_id)) {
    ut_a(s == space);
    ut_a(space->n_pending_ops == 0);
    ut_a(space->files.size() == 1);
    ut_a(space->files.front().n_pending == 0);

    space_detach(space);

    space_delete(space_id);

    mutex_release();

    space_free_low(space);
    ut_a(space == nullptr);

    if (!os_file_delete(innodb_data_file_key, path) &&
        !os_file_delete_if_exists(innodb_data_file_key, path, nullptr)) {
      /* Note: This is because we have removed the
      tablespace instance from the cache. */

      err = DB_IO_ERROR;
    }

  } else {
    mutex_release();

    err = DB_TABLESPACE_NOT_FOUND;
  }

  ut_free(path);

  clone_mark_active();

  return (err);
}





/** Truncate the tablespace to needed size.
@param[in]	space_id	Tablespace ID to truncate
@param[in]	size_in_pages	Truncate size.
@return true if truncate was successful. */
bool Fil_shard::space_truncate(space_id_t space_id, page_no_t size_in_pages) {
  /* Step-1: Prepare tablespace for truncate. This involves
  stopping all the new operations + IO on that tablespace
  and ensuring that related pages are flushed to disk. */
  if (space_prepare_for_truncate(space_id) != DB_SUCCESS) {
    return (false);
  }

#ifndef UNIV_HOTBACKUP
  /* Step-2: Invalidate buffer pool pages belonging to the tablespace
  to re-create. Remove all insert buffer entries for the tablespace */
  buf_LRU_flush_or_remove_pages(space_id, BUF_REMOVE_ALL_NO_WRITE, 0);
#endif /* !UNIV_HOTBACKUP */

  /* Step-3: Truncate the tablespace and accordingly update
  the fil_space_t handler that is used to access this tablespace. */
  mutex_acquire();

  fil_space_t *space = get_space_by_id(space_id);

  ut_a(space->files.size() == 1);

  fil_node_t &file = space->files.front();

  ut_ad(file.is_open);

  space->size = file.size = size_in_pages;

  bool success = os_file_truncate(file.name, file.handle, 0);

  if (success) {
    os_offset_t size = size_in_pages * UNIV_PAGE_SIZE;

    success = os_file_set_size(file.name, file.handle, 0, size,
                               srv_read_only_mode, true);

    if (success) {
      space->stop_new_ops = false;
    }
  }

  mutex_release();

  return (success);
}




#ifdef UNIV_DEBUG
/** Increase redo skipped count for a tablespace.
@param[in]	space_id	Tablespace ID */
void fil_space_inc_redo_skipped_count(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  ut_a(space != nullptr);

  ++space->redo_skipped_count;

  shard->mutex_release();
}

/** Decrease redo skipped count for a tablespace.
@param[in]	space_id	Tablespace id */
void fil_space_dec_redo_skipped_count(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  ut_a(space != nullptr);
  ut_a(space->redo_skipped_count > 0);

  --space->redo_skipped_count;

  shard->mutex_release();
}

/** Check whether a single-table tablespace is redo skipped.
@param[in]	space_id	Tablespace id
@return true if redo skipped */
bool fil_space_is_redo_skipped(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  ut_a(space != nullptr);

  bool is_redo_skipped = space->redo_skipped_count > 0;

  shard->mutex_release();

  return (is_redo_skipped);
}
#endif /* UNIV_DEBUG */

#ifndef UNIV_HOTBACKUP

/** Write redo log for renaming a file.
@param[in]	space_id	tablespace id
@param[in]	old_name	tablespace file name
@param[in]	new_name	tablespace file name after renaming
@param[in,out]	mtr		mini-transaction */
static void fil_name_write_rename(space_id_t space_id, const char *old_name,
                                  const char *new_name, mtr_t *mtr) {
  ut_ad(!fsp_is_system_or_temp_tablespace(space_id));
  ut_ad(!fsp_is_undo_tablespace(space_id));

  /* Note: A checkpoint can take place here. */

  DBUG_EXECUTE_IF("ib_crash_rename_log_1", DBUG_SUICIDE(););

  static const auto type = MLOG_FILE_RENAME;

  fil_op_write_log(type, space_id, old_name, new_name, 0, mtr);

  DBUG_EXECUTE_IF("ib_crash_rename_log_2", DBUG_SUICIDE(););

  /* Note: A checkpoint can take place here too before we
  have physically renamed the file. */
}

#endif /* !UNIV_HOTBACKUP */




/** Rename a single-table tablespace.
The tablespace must exist in the memory cache.
@param[in]	space_id	Tablespace ID
@param[in]	old_path	Old file name
@param[in]	new_name	New tablespace  name in the schema/space
@param[in]	new_path_in	New file name, or nullptr if it is located
                                in the normal data directory
@return InnoDB error code */
dberr_t Fil_shard::space_rename(space_id_t space_id, const char *old_path,
                                const char *new_name, const char *new_path_in) {
  fil_space_t *space;
  ulint count = 0;
  fil_node_t *file = nullptr;
  bool write_ddl_log = true;
  auto start_time = ut_time();

#ifdef UNIV_DEBUG
  static uint32_t crash_injection_rename_tablespace_counter = 1;
#endif /* UNIV_DEBUG */

  ut_a(space_id != TRX_SYS_SPACE);
  ut_ad(strchr(new_name, '/') != nullptr);

  for (;;) {
    bool retry = false;
    bool flush = false;

    ++count;

    if (!(count % 1000)) {
      ib::warn(ER_IB_MSG_295, old_path, ulong{space_id}, ulonglong{count});
    }

    /* The name map and space ID map are in the same shard. */
    mutex_acquire();

    space = get_space_by_id(space_id);

    DBUG_EXECUTE_IF("fil_rename_tablespace_failure_1", space = nullptr;);

    if (space == nullptr) {
      ib::error(ER_IB_MSG_296, ulong{space_id}, old_path);

      mutex_release();

      return (DB_ERROR);

    } else if (space->stop_ios) {
      /* Some other thread has stopped the IO. We need to
       wait for the other thread to complete its operation. */
      mutex_release();

      if (ut_time() - start_time >= PRINT_INTERVAL_SECS) {
        ib::warn(ER_IB_MSG_297);

        start_time = ut_time();
      }

      os_thread_sleep(1000000);

      continue;

    } else if (count > 25000) {
      mutex_release();

      return (DB_ERROR);

    } else if (space != get_space_by_name(space->name)) {
      ib::error(ER_IB_MSG_298, space->name);

      mutex_release();

      return (DB_ERROR);

    } else {
      auto new_space = get_space_by_name(new_name);

      if (new_space != nullptr) {
        if (new_space == space) {
          mutex_release();

          return (DB_SUCCESS);
        }

        ut_a(new_space->id == space->id);
      }
    }

    ut_a(space->files.size() == 1);

#ifndef UNIV_HOTBACKUP
    /* Don't write DDL log during recovery when log_ddl is
    not initialized. */

    if (write_ddl_log && log_ddl != nullptr) {
      /* Write ddl log when space->stop_ios is true
      can cause deadlock:
      a. buffer flush thread waits for rename thread to set
         stop_ios to false;
      b. rename thread waits for buffer flush thread to flush
         a page and release page lock. The page is ready for
         flush in double write buffer. */

      ut_ad(!space->stop_ios);

      file = &space->files.front();

      char *new_file_name = new_path_in == nullptr
                                ? Fil_path::make_ibd_from_table_name(new_name)
                                : mem_strdup(new_path_in);

      char *old_file_name = file->name;

      ut_ad(strchr(old_file_name, OS_PATH_SEPARATOR) != nullptr);

      ut_ad(strchr(new_file_name, OS_PATH_SEPARATOR) != nullptr);

      mutex_release();

      /* Rename ddl log is for rollback, so we exchange
      old file name with new file name. */
      dberr_t err = log_ddl->write_rename_space_log(space_id, new_file_name,
                                                    old_file_name);
      ut_free(new_file_name);
      if (err != DB_SUCCESS) {
        return (err);
      }

      write_ddl_log = false;
      continue;
    }
#endif /* !UNIV_HOTBACKUP */

    /* We temporarily close the .ibd file because we do
    not trust that operating systems can rename an open
    file. For the closing we have to wait until there
    are no pending I/O's or flushes on the file. */

    space->stop_ios = true;

    file = &space->files.front();

    if (file->n_pending > 0 || file->n_pending_flushes > 0 ||
        file->in_use > 0) {
      /* There are pending I/O's or flushes or the
      file is currently being extended, sleep for
      a while and retry */

      retry = true;

      space->stop_ios = false;

    } else if (file->modification_counter > file->flush_counter) {
      /* Flush the space */

      retry = flush = true;

      space->stop_ios = false;

    } else if (file->is_open) {
      close_file(file, false);
    }

    mutex_release();

    if (!retry) {
      ut_ad(space->stop_ios);
      break;
    }

    os_thread_sleep(100000);

    if (flush) {
      mutex_acquire();

      space_flush(space->id);

      mutex_release();
    }
  }

  ut_ad(space->stop_ios);

  char *new_file_name;

  if (new_path_in == nullptr) {
    new_file_name = Fil_path::make_ibd_from_table_name(new_name);
  } else {
    new_file_name = mem_strdup(new_path_in);
  }

  char *old_file_name = file->name;
  char *old_space_name = space->name;
  char *new_space_name = mem_strdup(new_name);

#ifndef UNIV_HOTBACKUP
  if (!recv_recovery_on) {
    mtr_t mtr;

    mtr.start();

    fil_name_write_rename(space_id, old_file_name, new_file_name, &mtr);

    mtr.commit();
  }
#endif /* !UNIV_HOTBACKUP */

  ut_ad(strchr(old_file_name, OS_PATH_SEPARATOR) != nullptr);
  ut_ad(strchr(new_file_name, OS_PATH_SEPARATOR) != nullptr);

  mutex_acquire();

  /* We already checked these. */
  ut_ad(space == get_space_by_name(old_space_name));
  ut_ad(get_space_by_name(new_space_name) == nullptr);

  bool success;

  DBUG_EXECUTE_IF("fil_rename_tablespace_failure_2", goto skip_rename;);

  DBUG_INJECT_CRASH("ddl_crash_before_rename_tablespace",
                    crash_injection_rename_tablespace_counter++);

  success = os_file_rename(innodb_data_file_key, old_file_name, new_file_name);

  DBUG_EXECUTE_IF("fil_rename_tablespace_failure_2", skip_rename
                  : success = false;);

  DBUG_INJECT_CRASH("ddl_crash_after_rename_tablespace",
                    crash_injection_rename_tablespace_counter++);

  if (success) {
    file->name = new_file_name;

    update_space_name_map(space, new_space_name);

    space->name = new_space_name;

  } else {
    /* Because nothing was renamed, we must free the new
    names, not the old ones. */
    old_file_name = new_file_name;
    old_space_name = new_space_name;
  }

  ut_ad(space->stop_ios);
  space->stop_ios = false;

  mutex_release();

  ut_free(old_file_name);
  ut_free(old_space_name);

  return (success ? DB_SUCCESS : DB_ERROR);
}






/** Create a IBD tablespace file.
@param[in]	space_id	Tablespace ID
@param[in]	name		Tablespace name in dbname/tablename format.
                                For general tablespaces, the 'dbname/' part
                                may be missing.
@param[in]	path		Path and filename of the datafile to create.
@param[in]	flags		Tablespace flags
@param[in]	size		Initial size of the tablespace file in pages,
                                must be >= FIL_IBD_FILE_INITIAL_SIZE
@return DB_SUCCESS or error code */
dberr_t fil_ibd_create(space_id_t space_id, const char *name, const char *path,
                       uint32_t flags, page_no_t size) {
  ut_a(size >= FIL_IBD_FILE_INITIAL_SIZE);
  ut_ad(!srv_read_only_mode);
  return (fil_create_tablespace(space_id, name, path, flags, size,
                                FIL_TYPE_TABLESPACE));
}

/** Create a session temporary tablespace (IBT) file.
@param[in]	space_id	Tablespace ID
@param[in]	name		Tablespace name
@param[in]	path		Path and filename of the datafile to create.
@param[in]	flags		Tablespace flags
@param[in]	size		Initial size of the tablespace file in pages,
                                must be >= FIL_IBT_FILE_INITIAL_SIZE
@return DB_SUCCESS or error code */
dberr_t fil_ibt_create(space_id_t space_id, const char *name, const char *path,
                       uint32_t flags, page_no_t size) {
  ut_a(size >= FIL_IBT_FILE_INITIAL_SIZE);
  return (fil_create_tablespace(space_id, name, path, flags, size,
                                FIL_TYPE_TEMPORARY));
}

#ifndef UNIV_HOTBACKUP
/** Open a single-table tablespace and optionally check the space id is
right in it. If not successful, print an error message to the error log. This
function is used to open a tablespace when we start up mysqld, and also in
IMPORT TABLESPACE.
NOTE that we assume this operation is used either at the database startup
or under the protection of the dictionary mutex, so that two users cannot
race here.

The fil_node_t::handle will not be left open.

@param[in]	validate	whether we should validate the tablespace
                                (read the first page of the file and
                                check that the space id in it matches id)
@param[in]	purpose		FIL_TYPE_TABLESPACE or FIL_TYPE_TEMPORARY
@param[in]	space_id	Tablespace ID
@param[in]	flags		tablespace flags
@param[in]	space_name	tablespace name of the datafile
                                If file-per-table, it is the table name in
                                the databasename/tablename format
@param[in]	table_name	table name in case if need to construct
                                file path
@param[in]	path_in		expected filepath, usually read from dictionary
@param[in]	strict		whether to report error when open ibd failed
@param[in]	old_space	whether it is a 5.7 tablespace opening
                                by upgrade
@return DB_SUCCESS or error code */
dberr_t fil_ibd_open(bool validate, fil_type_t purpose, space_id_t space_id,
                     uint32_t flags, const char *space_name,
                     const char *table_name, const char *path_in, bool strict,
                     bool old_space) {
  Datafile df;
  bool is_encrypted = FSP_FLAGS_GET_ENCRYPTION(flags);
  bool for_import = (purpose == FIL_TYPE_IMPORT);

  ut_ad(fil_type_is_data(purpose));

  if (!fsp_flags_is_valid(flags)) {
    return (DB_CORRUPTION);
  }

  /* Check if the file is already open. The space can be loaded
  via fil_space_get_first_path() on startup. This is a problem
  for partitioning code. It's a convoluted call graph via the DD.
  On Windows this can lead to a sharing violation when we attempt
  to open it again. */

  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  auto space = shard->get_space_by_id(space_id);

  if (space != nullptr) {
    shard->space_detach(space);
    shard->space_delete(space->id);
    shard->space_free_low(space);
    ut_a(space == nullptr);
  }

  shard->mutex_release();

  df.init(space_name, flags);

  if (path_in == nullptr) {
    df.make_filepath(nullptr, space_name, IBD);
  } else {
    df.set_filepath(path_in);
  }

  /* Attempt to open the tablespace. */
  if (df.open_read_only(strict) == DB_SUCCESS) {
    ut_ad(df.is_open());
  } else {
    ut_ad(!df.is_open());
    return (DB_CANNOT_OPEN_FILE);
  }

#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)
  const bool atomic_write =
      !srv_use_doublewrite_buf && fil_fusionio_enable_atomic_write(df.handle());
#else
  const bool atomic_write = false;
#endif /* !NO_FALLOCATE && UNIV_LINUX */

  dberr_t err;

  if ((validate || is_encrypted) &&
      (err = df.validate_to_dd(space_id, flags, for_import)) != DB_SUCCESS) {
    /* We don't reply the rename via the redo log anymore.
    Therefore we can get a space ID mismatch when validating
    the files during bootstrap. */

    if (!is_encrypted && err != DB_WRONG_FILE_NAME) {
      /* The following call prints an error message.
      For encrypted tablespace we skip print, since it should
      be keyring plugin issues. */
      os_file_get_last_error(true);

      ib::error(ER_IB_MSG_306, space_name, TROUBLESHOOT_DATADICT_MSG);
    }

    return (err);
  }

  /* If the encrypted tablespace is already opened,
  return success. */
  if (validate && is_encrypted && fil_space_get(space_id)) {
    return (DB_SUCCESS);
  }

  /* We pass UNINITIALIZED flags while we try to open DD tablesapce. In that
  case, set the flags now based on what is read from disk.*/
  if (FSP_FLAGS_ARE_NOT_SET(flags) && fsp_is_dd_tablespace(space_id)) {
    flags = df.flags();
    is_encrypted = FSP_FLAGS_GET_ENCRYPTION(flags);
  }

  space = fil_space_create(space_name, space_id, flags, purpose);

  if (space == nullptr) {
    return (DB_ERROR);
  }

  /* We do not measure the size of the file, that is why
  we pass the 0 below */

  const fil_node_t *file =
      shard->create_node(df.filepath(), 0, space, false, true, atomic_write);

  if (file == nullptr) {
    return (DB_ERROR);
  }

  if (validate && !old_space && !for_import) {
    if (df.server_version() > DD_SPACE_CURRENT_SRV_VERSION) {
      ib::error(ER_IB_MSG_1272, ulong{DD_SPACE_CURRENT_SRV_VERSION},
                ulonglong{df.server_version()});
      /* Server version is less than the tablespace server version.
      We don't support downgrade for 8.0 server, so report error */
      return (DB_SERVER_VERSION_LOW);
    }
    ut_ad(df.space_version() == DD_SPACE_CURRENT_SPACE_VERSION);
  }

  /* Set unencryption in progress flag */
  space->encryption_op_in_progress = df.m_encryption_op_in_progress;

  /* Its possible during Encryption processing, space flag for encryption
  has been updated in ibd file but server crashed before DD flags are
  updated. Thus, consider ibd setting too for encryption.

  It is safe because m_encryption_op_in_progress will be set to NONE
  always unless there is a crash before finishing Encryption. */
  if (space->encryption_op_in_progress == ENCRYPTION) {
    space->flags |= flags & FSP_FLAGS_MASK_ENCRYPTION;
  }

  /* For encryption tablespace, initialize encryption information.*/
  if ((is_encrypted || space->encryption_op_in_progress == ENCRYPTION) &&
      !for_import) {
    dberr_t err;
    byte *key = df.m_encryption_key;
    byte *iv = df.m_encryption_iv;

    ut_ad(key && iv);

    err = fil_set_encryption(space->id, Encryption::AES, key, iv);

    if (err != DB_SUCCESS) {
      return (DB_ERROR);
    }
  }

  return (DB_SUCCESS);
}

#else  /* !UNIV_HOTBACKUP */

/** Allocates a file name for an old version of a single-table tablespace.
The string must be freed by caller with ut_free()!
@param[in]	name		Original file name
@return own: file name */
static char *meb_make_ibbackup_old_name(const char *name) {
  char *path;
  ulint len = strlen(name);
  static const char suffix[] = "_ibbackup_old_vers_";

  path = static_cast<char *>(ut_malloc_nokey(len + 15 + sizeof(suffix)));

  memcpy(path, name, len);
  memcpy(path + len, suffix, sizeof(suffix) - 1);

  meb_sprintf_timestamp_without_extra_chars(path + len + sizeof(suffix) - 1);

  return (path);
}
#endif /* UNIV_HOTBACKUP */



/** Open an ibd tablespace and add it to the InnoDB data structures.
This is similar to fil_ibd_open() except that it is used while processing
the redo and DDL log, so the data dictionary is not available and very little
validation is done. The tablespace name is extracted from the
dbname/tablename.ibd portion of the filename, which assumes that the file
is a file-per-table tablespace.  Any name will do for now.  General
tablespace names will be read from the dictionary after it has been
recovered.  The tablespace flags are read at this time from the first page
of the file in validate_for_recovery().
@param[in]	space_id	tablespace ID
@param[in]	path		path/to/databasename/tablename.ibd
@param[out]	space		the tablespace, or nullptr on error
@return status of the operation */
fil_load_status Fil_shard::ibd_open_for_recovery(space_id_t space_id,
                                                 const std::string &path,
                                                 fil_space_t *&space) {
  /* If the a space is already in the file system cache with this
  space ID, then there is nothing to do. */

  mutex_acquire();

  space = get_space_by_id(space_id);

  mutex_release();

  const char *filename = path.c_str();

  if (space != nullptr) {
    ut_a(space->files.size() == 1);

    const auto &file = space->files.front();

    /* Compare the real paths. */
    if (Fil_path::equal(filename, file.name)) {
      return (FIL_LOAD_OK);
    }

#ifdef UNIV_HOTBACKUP
    ib::trace_2() << "Ignoring data file '" << filename << "' with space ID "
                  << space->id << ". Another data file called '" << file.name
                  << "' exists with the same space ID";
#else  /* UNIV_HOTBACKUP */
    ib::info(ER_IB_MSG_307, filename, ulong{space->id}, file.name);
#endif /* UNIV_HOTBACKUP */

    space = nullptr;

    return (FIL_LOAD_ID_CHANGED);
  }

  Datafile df;

  df.set_filepath(filename);

  if (df.open_read_only(false) != DB_SUCCESS) {
    return (FIL_LOAD_NOT_FOUND);
  }

  ut_ad(df.is_open());

  /* Read and validate the first page of the tablespace.
  Assign a tablespace name based on the tablespace type. */
  dberr_t err = df.validate_for_recovery(space_id);

  ut_a(err == DB_SUCCESS || err == DB_INVALID_ENCRYPTION_META);
  if (err == DB_INVALID_ENCRYPTION_META) {
    bool success = fil_system->erase(space_id);
    ut_a(success);
    return (FIL_LOAD_NOT_FOUND);
  }

  ut_a(df.space_id() == space_id);

  /* Get and test the file size. */
  os_offset_t size = os_file_get_size(df.handle());

  /* Every .ibd file is created >= 4 pages in size.
  Smaller files cannot be OK. */
  os_offset_t minimum_size;

  /* Every .ibd file is created >= FIL_IBD_FILE_INITIAL_SIZE
  pages in size. Smaller files cannot be OK. */
  {
    const page_size_t page_size(df.flags());

    minimum_size = FIL_IBD_FILE_INITIAL_SIZE * page_size.physical();
  }

  if (size == static_cast<os_offset_t>(-1)) {
    /* The following call prints an error message */
    os_file_get_last_error(true);

    ib::error(ER_IB_MSG_308) << "Could not measure the size of"
                                " single-table tablespace file '"
                             << df.filepath() << "'";

  } else if (size < minimum_size) {
#ifndef UNIV_HOTBACKUP
    ib::error(ER_IB_MSG_309)
        << "The size of tablespace file '" << df.filepath() << "' is only "
        << size << ", should be at least " << minimum_size << "!";
#else
    /* In MEB, we work around this error. */
    df.set_space_id(SPACE_UNKNOWN);
    df.set_flags(0);
#endif /* !UNIV_HOTBACKUP */
  }

  ut_ad(space == nullptr);

#ifdef UNIV_HOTBACKUP
  if (df.space_id() == SPACE_UNKNOWN || df.space_id() == 0) {
    char *new_path;

    ib::info(ER_IB_MSG_310)
        << "Renaming tablespace file '" << df.filepath() << "' with space ID "
        << df.space_id() << " to " << df.name()
        << "_ibbackup_old_vers_<timestamp>"
           " because its size "
        << df.size()
        << " is too small"
           " (< 4 pages 16 kB each), or the space id in the"
           " file header is not sensible. This can happen in"
           " an mysqlbackup run, and is not dangerous.";
    df.close();

    new_path = meb_make_ibbackup_old_name(df.filepath());

    bool success =
        os_file_rename(innodb_data_file_key, df.filepath(), new_path);

    ut_a(success);

    ut_free(new_path);

    return (FIL_LOAD_ID_CHANGED);
  }

  /* A backup may contain the same space several times, if the space got
  renamed at a sensitive time. Since it is enough to have one version of
  the space, we rename the file if a space with the same space id
  already exists in the tablespace memory cache. We rather rename the
  file than delete it, because if there is a bug, we do not want to
  destroy valuable data. */

  mutex_acquire();

  space = get_space_by_id(space_id);

  mutex_release();

  if (space != nullptr) {
    ib::info(ER_IB_MSG_311)
        << "Renaming data file '" << df.filepath() << "' with space ID "
        << space_id << " to " << df.name()
        << "_ibbackup_old_vers_<timestamp> because space " << space->name
        << " with the same id was scanned"
           " earlier. This can happen if you have renamed tables"
           " during an mysqlbackup run.";

    df.close();

    char *new_path = meb_make_ibbackup_old_name(df.filepath());

    bool success =
        os_file_rename(innodb_data_file_key, df.filepath(), new_path);

    ut_a(success);

    ut_free(new_path);
    return (FIL_LOAD_OK);
  }
#endif /* UNIV_HOTBACKUP */
  std::string tablespace_name;

#ifndef UNIV_HOTBACKUP
  dd_filename_to_spacename(df.name(), &tablespace_name);
#else
  /* During the apply-log operation, MEB already has translated the
  file name, so file name to space name conversin is not required. */

  tablespace_name = df.name();
#endif /* !UNIV_HOTBACKUP */

  fil_system->mutex_acquire_all();

  space = space_create(tablespace_name.c_str(), space_id, df.flags(),
                       FIL_TYPE_TABLESPACE);

  fil_system->mutex_release_all();

  if (space == nullptr) {
    return (FIL_LOAD_INVALID);
  }

  ut_ad(space->id == df.space_id());
  ut_ad(space->id == space_id);

  /* We do not use the size information we have about the file, because
  the rounding formula for extents and pages is somewhat complex; we
  let fil_node_create() do that task. */

  const fil_node_t *file;

  file = create_node(df.filepath(), 0, space, false, true, false);

  ut_a(file != nullptr);

  /* For encryption tablespace, initial encryption information. */
  if (FSP_FLAGS_GET_ENCRYPTION(space->flags) &&
      df.m_encryption_key != nullptr) {
    dberr_t err = fil_set_encryption(space->id, Encryption::AES,
                                     df.m_encryption_key, df.m_encryption_iv);

    if (err != DB_SUCCESS) {
      ib::error(ER_IB_MSG_312, space->name);
    }
  }

  /* Set unencryption in progress flag */
  space->encryption_op_in_progress = df.m_encryption_op_in_progress;

  return (FIL_LOAD_OK);
}

/** Open an ibd tablespace and add it to the InnoDB data structures.
This is similar to fil_ibd_open() except that it is used while processing
the redo log, so the data dictionary is not available and very little
validation is done. The tablespace name is extracted from the
dbname/tablename.ibd portion of the filename, which assumes that the file
is a file-per-table tablespace.  Any name will do for now.  General
tablespace names will be read from the dictionary after it has been
recovered.  The tablespace flags are read at this time from the first page
of the file in validate_for_recovery().
@param[in]	space_id	tablespace ID
@param[in]	path		path/to/databasename/tablename.ibd
@param[out]	space		the tablespace, or nullptr on error
@return status of the operation */
fil_load_status Fil_system::ibd_open_for_recovery(space_id_t space_id,
                                                  const std::string &path,
                                                  fil_space_t *&space) {
  /* System tablespace open should never come here. It should be
  opened explicitly using the config path. */
  ut_a(space_id != TRX_SYS_SPACE);

#ifndef UNIV_HOTBACKUP
  /* Do not attempt to open or load for recovery any undo tablespace that
  is currently being truncated. */
  if (fsp_is_undo_tablespace(space_id) &&
      undo::is_active_truncate_log_present(undo::id2num(space_id))) {
    return (FIL_LOAD_NOT_FOUND);
  }
#endif /* !UNIV_HOTBACKUP */

  auto shard = shard_by_id(space_id);

  return (shard->ibd_open_for_recovery(space_id, path, space));
}

#ifndef UNIV_HOTBACKUP

/** Report that a tablespace for a table was not found.
@param[in]	name		Table name
@param[in]	space_id	Table's space ID */
static void fil_report_missing_tablespace(const char *name,
                                          space_id_t space_id) {
  ib::error(ER_IB_MSG_313)
      << "Table " << name << " in the InnoDB data dictionary has tablespace id "
      << space_id
      << ","
         " but tablespace with that id or name does not exist. Have"
         " you deleted or moved .ibd files?";
}

bool Fil_shard::adjust_space_name(fil_space_t *space,
                                  const char *dd_space_name) {
  if (!strcmp(space->name, dd_space_name)) {
    return (true);
  }

  bool replace_general =
      FSP_FLAGS_GET_SHARED(space->flags) &&
      0 == strncmp(space->name, general_space_name, strlen(general_space_name));
  bool replace_undo =
      fsp_is_undo_tablespace(space->id) &&
      0 == strncmp(space->name, undo_space_name, strlen(undo_space_name));

  /* Update the auto-generated fil_space_t::name */
  if (replace_general || replace_undo) {
    char *old_space_name = space->name;
    char *new_space_name = mem_strdup(dd_space_name);

    update_space_name_map(space, new_space_name);

    space->name = new_space_name;

    ut_free(old_space_name);
  }

  /* Update the undo::Tablespace::name. Since the fil_shard mutex is held by
  the caller, it would be a sync order violation to get undo::spaces->s_lock.
  It is OK to skip this s_lock since this occurs during boot_tablespaces()
  which is still single threaded. */
  if (replace_undo) {
    space_id_t space_num = undo::id2num(space->id);
    undo::Tablespace *undo_space = undo::spaces->find(space_num);
    undo_space->set_space_name(dd_space_name);
  }

  return (replace_general || replace_undo);
}

/** Returns true if a matching tablespace exists in the InnoDB tablespace
memory cache.
@param[in]	space_id		Tablespace ID
@param[in]	name			Tablespace name used in
                                        fil_space_create().
@param[in]	print_err		Print detailed error information to the
                                        error log if a matching tablespace is
                                        not found from memory.
@param[in]	adjust_space		Whether to adjust space id on mismatch
@param[in]	heap			Heap memory
@param[in]	table_id		table id
@return true if a matching tablespace exists in the memory cache */
bool Fil_shard::space_check_exists(space_id_t space_id, const char *name,
                                   bool print_err, bool adjust_space,
                                   mem_heap_t *heap, table_id_t table_id) {
  fil_space_t *fnamespace = nullptr;

  mutex_acquire();

  /* Look if there is a space with the same id */
  fil_space_t *space = get_space_by_id(space_id);

  /* name is nullptr when replaying a DELETE ddl log. */
  if (name == nullptr) {
    mutex_release();
    return (space != nullptr);
  }

  if (space != nullptr) {
    /* No need to check a general tablespace name if the DD
    is not yet available. */
    if (!srv_sys_tablespaces_open && FSP_FLAGS_GET_SHARED(space->flags)) {
      mutex_release();
      return (true);
    }

    /* Sometimes the name has been auto-generated when the
    datafile is discovered and needs to be adjusted to that
    of the DD. This happens for general and undo tablespaces. */
    if (srv_sys_tablespaces_open && adjust_space &&
        adjust_space_name(space, name)) {
      mutex_release();
      return (true);
    }

    /* If this space has the expected name, use it. */
    fnamespace = get_space_by_name(name);

    if (space == fnamespace) {
      /* Found */
      mutex_release();
      return (true);
    }
  }

  /* Info from "fnamespace" comes from the ibd file itself, it can
  be different from data obtained from System tables since file
  operations are not transactional. If adjust_space is set, and the
  mismatching space are between a user table and its temp table, we
  shall adjust the ibd file name according to system table info */
  if (adjust_space && space != nullptr &&
      row_is_mysql_tmp_table_name(space->name) &&
      !row_is_mysql_tmp_table_name(name)) {
    /* Atomic DDL's "ddl_log" will adjust the tablespace name. */
    mutex_release();

    return (true);

  } else if (!print_err) {
    ;

  } else if (space == nullptr) {
    if (fnamespace == nullptr) {
      if (print_err) {
        fil_report_missing_tablespace(name, space_id);
      }

    } else {
      ib::error(ER_IB_MSG_314)
          << "Table " << name
          << " in InnoDB data"
             " dictionary has tablespace id "
          << space_id
          << ", but a tablespace with that id does not"
             " exist. There is a tablespace of name "
          << fnamespace->name << " and id " << fnamespace->id
          << ", though. Have you"
             " deleted or moved .ibd files?";
    }

    ib::warn(ER_IB_MSG_315) << TROUBLESHOOT_DATADICT_MSG;

  } else if (0 != strcmp(space->name, name)) {
    ib::error(ER_IB_MSG_316) << "Table " << name
                             << " in InnoDB data dictionary"
                                " has tablespace id "
                             << space_id
                             << ", but the"
                                " tablespace with that id has name "
                             << space->name
                             << ". Have you deleted or moved .ibd"
                                " files?";

    if (fnamespace != nullptr) {
      ib::error(ER_IB_MSG_317) << "There is a tablespace with the right"
                                  " name: "
                               << fnamespace->name
                               << ", but its id"
                                  " is "
                               << fnamespace->id << ".";
    }

    ib::warn(ER_IB_MSG_318) << TROUBLESHOOT_DATADICT_MSG;
  }

  mutex_release();

  return (false);
}


#endif /* !UNIV_HOTBACKUP */





#ifdef UNIV_HOTBACKUP
/** Extends all tablespaces to the size stored in the space header. During the
mysqlbackup --apply-log phase we extended the spaces on-demand so that log
records could be applied, but that may have left spaces still too small
compared to the size stored in the space header. */
void Fil_shard::meb_extend_tablespaces_to_stored_len() {
  ut_ad(mutex_owned());

  byte *buf = static_cast<byte *>(ut_malloc_nokey(UNIV_PAGE_SIZE));

  ut_a(buf != nullptr);

  for (auto &elem : m_spaces) {
    auto space = elem.second;

    ut_a(space->purpose == FIL_TYPE_TABLESPACE);

    /* No need to protect with a mutex, because this is
    a single-threaded operation */

    mutex_release();

    dberr_t error;

    const page_size_t page_size(space->flags);

    error = fil_read(page_id_t(space->id, 0), page_size, 0,
                     page_size.physical(), buf);

    ut_a(error == DB_SUCCESS);

    ulint size_in_header;

    size_in_header = fsp_header_get_field(buf, FSP_SIZE);

    bool success;

    success = space_extend(space, size_in_header);

    if (!success) {
      ib::error(ER_IB_MSG_321)
          << "Could not extend the tablespace of " << space->name
          << " to the size stored in"
             " header, "
          << size_in_header
          << " pages;"
             " size after extension "
          << 0
          << " pages. Check that you have free disk"
             " space and retry!";

      ut_a(success);
    }

    mutex_acquire();
  }

  ut_free(buf);
}

/** Extends all tablespaces to the size stored in the space header. During the
mysqlbackup --apply-log phase we extended the spaces on-demand so that log
records could be applied, but that may have left spaces still too small
compared to the size stored in the space header. */
void meb_extend_tablespaces_to_stored_len() {
  fil_system->meb_extend_tablespaces_to_stored_len();
}

bool meb_is_redo_log_only_restore = false;

/** Determine if file is intermediate / temporary. These files are
created during reorganize partition, rename tables, add / drop columns etc.
@param[in]	filepath	absolute / relative or simply file name
@retvalue	true		if it is intermediate file
@retvalue	false		if it is normal file */
bool meb_is_intermediate_file(const std::string &filepath) {
  std::string file_name = filepath;

  {
    /** If its redo only restore, apply log needs to got through the
        intermediate steps to apply a ddl.
        Some of these operation might result in intermediate files.
    */
    if (meb_is_redo_log_only_restore) return false;
    /* extract file name from relative or absolute file name */
    auto pos = file_name.rfind(OS_PATH_SEPARATOR);

    if (pos != std::string::npos) {
      ++pos;
      file_name = file_name.substr(pos);
    }
  }

  transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);

  if (file_name[0] != '#') {
    auto pos = file_name.rfind("#tmp#.ibd");
    if (pos != std::string::npos) {
      return (true);
    } else {
      return (false); /* normal file name */
    }
  }

  static std::vector<std::string> prefixes = {"#sql-", "#sql2-", "#tmp#",
                                              "#ren#"};

  /* search for the unsupported patterns */
  for (const auto &prefix : prefixes) {
    if (Fil_path::has_prefix(file_name, prefix)) {
      return (true);
    }
  }

  return (false);
}

/** Return the space ID based of the remote general tablespace name.
This is a wrapper over fil_space_get_id_by_name() method. it means,
the tablespace must be found in the tablespace memory cache.
This method extracts the tablespace name from input parameters and checks if
it has been loaded in memory cache through either any of the remote general
tablespaces directories identified at the time memory cache created.
@param[in, out]	tablespace	Tablespace name
@return space ID if tablespace found, SPACE_UNKNOWN if not found. */
space_id_t meb_fil_space_get_rem_gen_ts_id_by_name(std::string &tablespace) {
  space_id_t space_id = SPACE_UNKNOWN;

  for (auto newpath : rem_gen_ts_dirs) {
    auto pos = tablespace.rfind(OS_PATH_SEPARATOR);

    if (pos == std::string::npos) {
      break;
    }

    newpath += tablespace.substr(pos);

    space_id = fil_space_get_id_by_name(newpath.c_str());

    if (space_id != SPACE_UNKNOWN) {
      tablespace = newpath;
      break;
    }
  }

  return (space_id);
}

/** Tablespace item during recovery */
struct MEB_file_name {
  /** Constructor */
  MEB_file_name(std::string name, bool deleted)
      : m_name(name), m_space(), m_deleted(deleted) {}

  /** Tablespace file name (MLOG_FILE_NAME) */
  std::string m_name;

  /** Tablespace object (NULL if not valid or not found) */
  fil_space_t *m_space;

  /** Whether the tablespace has been deleted */
  bool m_deleted;
};

/** Map of dirty tablespaces during recovery */
using MEB_recv_spaces =
    std::map<space_id_t, MEB_file_name, std::less<space_id_t>,
             ut_allocator<std::pair<const space_id_t, MEB_file_name>>>;

static MEB_recv_spaces recv_spaces;

/** Checks if MEB has loaded this space for reovery.
@param[in]	space_id	Tablespace ID
@return true if the space_id is loaded */
bool meb_is_space_loaded(const space_id_t space_id) {
  return (recv_spaces.find(space_id) != recv_spaces.end());
}

/** Set the keys for an encrypted tablespace.
@param[in]	space		Tablespace for which to set the key */
static void meb_set_encryption_key(const fil_space_t *space) {
  ut_ad(FSP_FLAGS_GET_ENCRYPTION(space->flags));

  for (auto &key : *recv_sys->keys) {
    if (key.space_id != space->id) {
      continue;
    }

    dberr_t err;

    err = fil_set_encryption(space->id, Encryption::AES, key.ptr, key.iv);

    if (err != DB_SUCCESS) {
      ib::error(ER_IB_MSG_322) << "Can't set encryption information"
                               << " for tablespace" << space->name << "!";
    }

    ut_free(key.iv);
    ut_free(key.ptr);

    key.iv = nullptr;
    key.ptr = nullptr;
    key.space_id = 0;
  }
}

/** Process a file name passed as an input
Wrapper around meb_name_process()
@param[in,out]	name		absolute path of tablespace file
@param[in]	space_id	The tablespace ID
@param[in]	deleted		true if MLOG_FILE_DELETE */
void Fil_system::meb_name_process(char *name, space_id_t space_id,
                                  bool deleted) {
  ut_ad(space_id != TRX_SYS_SPACE);

  /* We will also insert space=nullptr into the map, so that
  further checks can ensure that a MLOG_FILE_NAME record was
  scanned before applying any page records for the space_id. */

  Fil_path::normalize(name);

  size_t len = std::strlen(name);

  MEB_file_name fname(std::string(name, len - 1), deleted);

  auto p = recv_spaces.insert(std::make_pair(space_id, fname));

  ut_ad(p.first->first == space_id);

  MEB_file_name &f = p.first->second;

  if (deleted) {
    /* Got MLOG_FILE_DELETE */

    if (!p.second && !f.m_deleted) {
      f.m_deleted = true;

      if (f.m_space != nullptr) {
        f.m_space = nullptr;
      }
    }

    ut_ad(f.m_space == nullptr);

  } else if (p.second || f.m_name != fname.m_name) {
    fil_space_t *space;

    /* Check if the tablespace file exists and contains
    the space_id. If not, ignore the file after displaying
    a note. Abort if there are multiple files with the
    same space_id. */

    switch (ibd_open_for_recovery(space_id, name, space)) {
      case FIL_LOAD_OK:
        ut_ad(space != nullptr);

        /* For encrypted tablespace, set key and iv. */
        if (FSP_FLAGS_GET_ENCRYPTION(space->flags) &&
            recv_sys->keys != nullptr) {
          meb_set_encryption_key(space);
        }

        if (f.m_space == nullptr || f.m_space == space) {
          f.m_name = fname.m_name;
          f.m_space = space;
          f.m_deleted = false;

        } else {
          ib::error(ER_IB_MSG_323)
              << "Tablespace " << space_id << " has been found in two places: '"
              << f.m_name << "' and '" << name
              << "'."
                 " You must delete one of them.";

          recv_sys->found_corrupt_fs = true;
        }
        break;

      case FIL_LOAD_ID_CHANGED:
        ut_ad(space == nullptr);

        ib::trace_1() << "Ignoring file " << name << " for space-id mismatch "
                      << space_id;
        break;

      case FIL_LOAD_NOT_FOUND:
        /* No matching tablespace was found; maybe it
        was renamed, and we will find a subsequent
        MLOG_FILE_* record. */
        ut_ad(space == nullptr);
        break;

      case FIL_LOAD_INVALID:
        ut_ad(space == nullptr);

        ib::warn(ER_IB_MSG_324) << "Invalid tablespace " << name;
        break;

      case FIL_LOAD_MISMATCH:
        ut_ad(space == nullptr);
        break;
    }
  }
}

/** Process a file name passed as an input
Wrapper around meb_name_process()
@param[in]	name		absolute path of tablespace file
@param[in]	space_id	the tablespace ID */
void meb_fil_name_process(const char *name, space_id_t space_id) {
  char *file_name = static_cast<char *>(mem_strdup(name));

  fil_system->meb_name_process(file_name, space_id, false);

  ut_free(file_name);
}

/** Test, if a file path name contains a back-link ("../").
We assume a path to a file. So we don't check for a trailing "/..".
@param[in]	path		path to check
@return	whether the path contains a back-link.
 */
static bool meb_has_back_link(const std::string &path) {
#ifdef _WIN32
  static const std::string DOT_DOT_SLASH = "..\\";
  static const std::string SLASH_DOT_DOT_SLASH = "\\..\\";
#else
  static const std::string DOT_DOT_SLASH = "../";
  static const std::string SLASH_DOT_DOT_SLASH = "/../";
#endif /* _WIN32 */
  return ((0 == path.compare(0, 3, DOT_DOT_SLASH)) ||
          (std::string::npos != path.find(SLASH_DOT_DOT_SLASH)));
}

/** Parse a file name retrieved from a MLOG_FILE_* record,
and return the absolute file path corresponds to backup dir
as well as in the form of database/tablespace
@param[in]	file_name	path emitted by the redo log
@param[in]	flags		flags emitted by the redo log
@param[in]	space_id	space_id emmited by the redo log
@param[out]	absolute_path	absolute path of tablespace
corresponds to target dir
@param[out]	tablespace_name	name in the form of database/table */
static void meb_make_abs_file_path(const std::string &name, uint32_t flags,
                                   space_id_t space_id,
                                   std::string &absolute_path,
                                   std::string &tablespace_name) {
  Datafile df;
  std::string file_name = name;

  /* If the tablespace path name is absolute or has back-links ("../"),
  we assume, that it is located outside of datadir. */
  if (Fil_path::is_absolute_path(file_name.c_str()) ||
      (meb_has_back_link(file_name) && !replay_in_datadir)) {
    if (replay_in_datadir) {
      /* This is an apply-log in the restored datadir. Take the path as is. */
      df.set_filepath(file_name.c_str());
    } else {
      /* This is an apply-log in backup_dir/datadir. Get the file inside. */
      auto pos = file_name.rfind(OS_PATH_SEPARATOR);

      /* if it is file per tablespace, then include the schema
      directory as well */
      if (fsp_is_file_per_table(space_id, flags) && pos != std::string::npos) {
        pos = file_name.rfind(OS_PATH_SEPARATOR, pos - 1);
      }

      if (pos == std::string::npos) {
        ib::fatal(ER_IB_MSG_325)
            << "Could not extract the tabelspace"
            << " file name from the in the path : " << name;
      }

      ++pos;

      file_name = file_name.substr(pos);

      df.make_filepath(MySQL_datadir_path, file_name.c_str(), IBD);
    }

  } else {
    /* This is an apply-log with a relative path, either in the restored
    datadir, or in backup_dir/datadir. If in the restored datadir, the
    path might start with "../" to reach outside of datadir. */
    auto pos = file_name.find(OS_PATH_SEPARATOR);

    /* Remove the cur dir from the path as this will cause the
    path name mismatch when we try to find out the space_id based
    on tablespace name */

    if (file_name.substr(0, pos) == ".") {
      ++pos;
      file_name = file_name.substr(pos);
    }

    /* make_filepath() does not prepend the directory, if the file name
    starts with "../". Prepend it unconditionally here. */
    file_name.insert(0, 1, OS_PATH_SEPARATOR);
    file_name.insert(0, MySQL_datadir_path);

    df.make_filepath(nullptr, file_name.c_str(), IBD);
  }

  df.set_flags(flags);
  df.set_space_id(space_id);
  df.set_name(nullptr);

  absolute_path = df.filepath();

  tablespace_name = df.name();
}

/** Process a MLOG_FILE_CREATE redo record.
@param[in]	page_id		Page id of the redo log record
@param[in]	flags		Tablespace flags
@param[in]	name		Tablespace filename */
static void meb_tablespace_redo_create(const page_id_t &page_id, uint32_t flags,
                                       const char *name) {
  std::string abs_file_path;
  std::string tablespace_name;

  meb_make_abs_file_path(name, flags, page_id.space(), abs_file_path,
                         tablespace_name);

  if (!meb_replay_file_ops || meb_is_intermediate_file(abs_file_path.c_str()) ||
      fil_space_get(page_id.space()) ||
      fil_space_get_id_by_name(tablespace_name.c_str()) != SPACE_UNKNOWN ||
      meb_fil_space_get_rem_gen_ts_id_by_name(tablespace_name) !=
          SPACE_UNKNOWN) {
    /* Don't create table while :-
    1. scanning the redo logs during backup
    2. apply-log on a partial backup
    3. if it is intermediate file
    4. tablespace is already loaded in memory
    5. tablespace is a remote general tablespace which is
       already loaded for recovery/apply-log from different
       directory path */

    ib::trace_1() << "Ignoring the log record. No need to "
                  << "create the tablespace : " << abs_file_path;
  } else {
    auto it = recv_spaces.find(page_id.space());

    if (it == recv_spaces.end() || it->second.m_name != abs_file_path) {
      ib::trace_1() << "Creating the tablespace : " << abs_file_path
                    << ", space_id : " << page_id.space();

      dberr_t ret = fil_ibd_create(page_id.space(), tablespace_name.c_str(),
                                   abs_file_path.c_str(), flags,
                                   FIL_IBD_FILE_INITIAL_SIZE);

      if (ret != DB_SUCCESS) {
        ib::fatal(ER_IB_MSG_326)
            << "Could not create the tablespace : " << abs_file_path
            << " with space Id : " << page_id.space();
      }
    }
  }
}

/** Process a MLOG_FILE_RENAME redo record.
@param[in]	page_id		Page id of the redo log record
@param[in]	from_name	Tablespace from filename
@param[in]	to_name		Tablespace to filename */
static void meb_tablespace_redo_rename(const page_id_t &page_id,
                                       const char *from_name,
                                       const char *to_name) {
  std::string abs_to_path;
  std::string abs_from_path;
  std::string tablespace_name;

  meb_make_abs_file_path(from_name, 0, page_id.space(), abs_from_path,
                         tablespace_name);

  meb_make_abs_file_path(to_name, 0, page_id.space(), abs_to_path,
                         tablespace_name);

  char *new_name = nullptr;

  if (!meb_replay_file_ops || meb_is_intermediate_file(from_name) ||
      meb_is_intermediate_file(to_name) ||
      fil_space_get_id_by_name(tablespace_name.c_str()) != SPACE_UNKNOWN ||
      meb_fil_space_get_rem_gen_ts_id_by_name(tablespace_name) !=
          SPACE_UNKNOWN ||
      fil_space_get(page_id.space()) == nullptr) {
    /* Don't rename table while :
    1. Scanning the redo logs during backup
    2. Apply-log on a partial backup
    3. Either of old or new tables are intermediate table
    4. The new name is already loaded for recovery/apply-log
    5. The new name is a remote general tablespace which is
       already loaded for recovery/apply-log from different
       directory path
    6. Tablespace is not yet loaded in memory.
    This will prevent unintended renames during recovery. */

    ib::trace_1() << "Ignoring the log record. "
                  << "No need to rename tablespace";

    return;

  } else {
    ib::trace_1() << "Renaming space id : " << page_id.space()
                  << ", old tablespace name : " << from_name
                  << " to new tablespace name : " << to_name;

    new_name = static_cast<char *>(mem_strdup(abs_to_path.c_str()));
  }

  meb_fil_name_process(from_name, page_id.space());
  meb_fil_name_process(new_name, page_id.space());

  if (meb_replay_file_ops) {
    if (!fil_op_replay_rename(page_id, abs_from_path.c_str(),
                              abs_to_path.c_str())) {
      recv_sys->found_corrupt_fs = true;
    }

    meb_fil_name_process(to_name, page_id.space());
  }

  ut_free(new_name);
}

/** Process a MLOG_FILE_DELETE redo record.
@param[in]	page_id		Page id of the redo log record
@param[in]	name		Tablespace filename */
static void meb_tablespace_redo_delete(const page_id_t &page_id,
                                       const char *name) {
  std::string abs_file_path;
  std::string tablespace_name;

  meb_make_abs_file_path(name, 0, page_id.space(), abs_file_path,
                         tablespace_name);

  char *file_name = static_cast<char *>(mem_strdup(name));

  fil_system->meb_name_process(file_name, page_id.space(), true);

  if (meb_replay_file_ops && fil_space_get(page_id.space())) {
    dberr_t err =
        fil_delete_tablespace(page_id.space(), BUF_REMOVE_FLUSH_NO_WRITE);

    ut_a(err == DB_SUCCESS);
  }

  ut_free(file_name);
}

#endif /* UNIV_HOTBACKUP */

/*========== RESERVE FREE EXTENTS (for a B-tree split, for example) ===*/






/*============================ FILE I/O ================================*/







/** Report information about an invalid page access.
@param[in]	block_offset	Block offset
@param[in]	space_id	Tablespace ID
@param[in]	space_name	Tablespace name
@param[in]	byte_offset	Byte offset
@param[in]	len		I/O length
@param[in]	is_read		I/O type
@param[in]	line		Line called from */
static void fil_report_invalid_page_access_low(page_no_t block_offset,
                                               space_id_t space_id,
                                               const char *space_name,
                                               ulint byte_offset, ulint len,
                                               bool is_read, int line) {
  ib::error(ER_IB_MSG_328)
      << "Trying to access page number " << block_offset
      << " in"
         " space "
      << space_id << ", space name " << space_name
      << ","
         " which is outside the tablespace bounds. Byte offset "
      << byte_offset << ", len " << len << ", i/o type "
      << (is_read ? "read" : "write")
      << ". If you get this error at mysqld startup, please check"
         " that your my.cnf matches the ibdata files that you have in"
         " the MySQL server.";

  ib::error(ER_IB_MSG_329) << "Server exits"
#ifdef UNIV_DEBUG
                           << " at "
                           << "fil0fil.cc"
                           << "[" << line << "]"
#endif /* UNIV_DEBUG */
                           << ".";

  ut_error;
}

#define fil_report_invalid_page_access(b, s, n, o, l, t) \
  fil_report_invalid_page_access_low((b), (s), (n), (o), (l), (t), __LINE__)

/** Set encryption information for IORequest.
@param[in,out]	req_type	IO request
@param[in]	page_id		page id
@param[in]	space		table space */
void fil_io_set_encryption(IORequest &req_type, const page_id_t &page_id,
                           fil_space_t *space) {
  /* Don't encrypt page 0 of all tablespaces except redo log
  tablespace, all pages from the system tablespace. */
  if (space->encryption_type == Encryption::NONE ||
      (space->encryption_op_in_progress == UNENCRYPTION &&
       req_type.is_write()) ||
      (page_id.page_no() == 0 && !req_type.is_log())) {
    req_type.clear_encrypted();
    return;
  }

  /* For writting redo log, if encryption for redo log is disabled,
  skip set encryption. */
  if (req_type.is_log() && req_type.is_write() && !srv_redo_log_encrypt) {
    req_type.clear_encrypted();
    return;
  }

  /* For writting undo log, if encryption for undo log is disabled,
  skip set encryption. */
  if (fsp_is_undo_tablespace(space->id) && !srv_undo_log_encrypt &&
      req_type.is_write()) {
    req_type.clear_encrypted();
    return;
  }

  /* Make any active clone operation to abort, in case
  log encryption is set after clone operation is started. */
  clone_mark_abort(true);
  clone_mark_active();

  req_type.encryption_key(space->encryption_key, space->encryption_klen,
                          space->encryption_iv);

  req_type.encryption_algorithm(Encryption::AES);
}



/** Get the file name for IO and the local offset within that file.
@param[in]	req_type	IO context
@param[in,out]	space		Tablespace for IO
@param[in,out]	page_no		The relative page number in the file
@param[out]	file		File node
@return DB_SUCCESS or error code */
dberr_t Fil_shard::get_file_for_io(const IORequest &req_type,
                                   fil_space_t *space, page_no_t *page_no,
                                   fil_node_t *&file) {
  if (space->files.size() > 1) {
    ut_a(space->id == TRX_SYS_SPACE || space->purpose == FIL_TYPE_TEMPORARY ||
         space->id == dict_sys_t_s_log_space_first_id);

    for (auto &f : space->files) {
      if (f.size > *page_no) {
        file = &f;
        return (DB_SUCCESS);
      }

      *page_no -= f.size;
    }

  } else if (!space->files.empty()) {
    fil_node_t &f = space->files.front();

    if ((fsp_is_ibd_tablespace(space->id) && f.size == 0) ||
        f.size > *page_no) {
      /* We do not know the size of a single-table tablespace
      before we open the file */

      file = &f;

      return (DB_SUCCESS);

    } else {
#ifndef UNIV_HOTBACKUP
      if (space->id != TRX_SYS_SPACE && req_type.is_read() &&
          !undo::is_active(space->id)) {
        file = nullptr;

        /* Page access request for a page that is
        outside the truncated UNDO tablespace bounds. */

        return (DB_TABLE_NOT_FOUND);
      }
#else  /* !UNIV_HOTBACKUP */
    /* In backup, is_under_construction() is always false */
#endif /* !UNIV_HOTBACKUP */
    }
  }

  file = nullptr;
  return (DB_ERROR);
}

#ifndef UNIV_HOTBACKUP
/** Read or write log file data synchronously.
@param[in]	type		IO context
@param[in]	page_id		page id
@param[in]	page_size	page size
@param[in]	byte_offset	remainder of offset in bytes; in AIO
                                this must be divisible by the OS block
                                size
@param[in]	len		how many bytes to read or write; this
                                must not cross a file boundary; in AIO
                                this must be a block size multiple
@param[in,out]	buf		buffer where to store read data or
                                from where to write
@return error code
@retval DB_SUCCESS on success */
dberr_t Fil_shard::do_redo_io(const IORequest &type, const page_id_t &page_id,
                              const page_size_t &page_size, ulint byte_offset,
                              ulint len, void *buf) {
  IORequest req_type(type);

  ut_ad(len > 0);
  ut_ad(req_type.is_log());
  ut_ad(req_type.validate());
  ut_ad(fil_validate_skip());
  ut_ad(byte_offset < UNIV_PAGE_SIZE);
  ut_ad(UNIV_PAGE_SIZE == (ulong)(1 << UNIV_PAGE_SIZE_SHIFT));

  if (req_type.is_read()) {
    srv_stats.data_read.add(len);

  } else if (req_type.is_write()) {
    ut_ad(!srv_read_only_mode);
    srv_stats.data_written.add(len);
  }

  fil_space_t *space = get_space_by_id(page_id.space());

  fil_node_t *file;
  page_no_t page_no = page_id.page_no();
  dberr_t err = get_file_for_io(req_type, space, &page_no, file);

  ut_a(file != nullptr);
  ut_a(err == DB_SUCCESS);
  ut_a(page_size.physical() == page_size.logical());

  os_offset_t offset = (os_offset_t)page_no * page_size.physical();

  offset += byte_offset;

  ut_a(file->size - page_no >=
       (byte_offset + len + (page_size.physical() - 1)) / page_size.physical());

  ut_a((len % OS_FILE_LOG_BLOCK_SIZE) == 0);
  ut_a(byte_offset % OS_FILE_LOG_BLOCK_SIZE == 0);

  /* Set encryption information. */
  fil_io_set_encryption(req_type, page_id, space);

  req_type.block_size(file->block_size);

  if (!file->is_open) {
    ut_a(file->n_pending == 0);

    bool success = open_file(file, false);

    ut_a(success);
  }

  if (req_type.is_read()) {
    err = os_file_read(req_type, file->handle, buf, offset, len);

  } else {
    ut_ad(!srv_read_only_mode);

    err = os_file_write(req_type, file->name, file->handle, buf, offset, len);
  }

  if (type.is_write()) {
    mutex_acquire();

    ++m_modification_counter;

    file->modification_counter = m_modification_counter;

    add_to_unflushed_list(file->space);

    mutex_release();
  }

  return (err);
}
#endif /* !UNIV_HOTBACKUP */

/** Read or write data. This operation could be asynchronous (aio).
@param[in]	type		IO context
@param[in]	sync		whether synchronous aio is desired
@param[in]	page_id		page id
@param[in]	page_size	page size
@param[in]	byte_offset	remainder of offset in bytes; in aio this
                                must be divisible by the OS block size
@param[in]	len		how many bytes to read or write; this must
                                not cross a file boundary; in AIO this must
                                be a block size multiple
@param[in,out]	buf		buffer where to store read data or from where
                                to write; in aio this must be appropriately
                                aligned
@param[in]	message		message for aio handler if !sync, else ignored
@return error code
@retval DB_SUCCESS on success
@retval DB_TABLESPACE_DELETED if the tablespace does not exist */
dberr_t Fil_shard::do_io(const IORequest &type, bool sync,
                         const page_id_t &page_id, const page_size_t &page_size,
                         ulint byte_offset, ulint len, void *buf,
                         void *message) {
  IORequest req_type(type);

  ut_ad(req_type.validate());

  ut_ad(len > 0);
  ut_ad(byte_offset < UNIV_PAGE_SIZE);
  ut_ad(!page_size.is_compressed() || byte_offset == 0);
  ut_ad(UNIV_PAGE_SIZE == (ulong)(1 << UNIV_PAGE_SIZE_SHIFT));

  ut_ad(fil_validate_skip());

#ifndef UNIV_HOTBACKUP
  /* ibuf bitmap pages must be read in the sync AIO mode: */
  ut_ad(recv_no_ibuf_operations || req_type.is_write() ||
        !ibuf_bitmap_page(page_id, page_size) || sync || req_type.is_log());

  AIO_mode aio_mode = get_AIO_mode(req_type, sync);

  if (req_type.is_read()) {
    srv_stats.data_read.add(len);

    if (aio_mode == AIO_mode::NORMAL && !recv_no_ibuf_operations &&
        ibuf_page(page_id, page_size, nullptr)) {
      /* Reduce probability of deadlock bugs
      in connection with ibuf: do not let the
      ibuf I/O handler sleep */

      req_type.clear_do_not_wake();

      aio_mode = AIO_mode::IBUF;
    }

  } else if (req_type.is_write()) {
    ut_ad(!srv_read_only_mode || fsp_is_system_temporary(page_id.space()));

    srv_stats.data_written.add(len);
  }
#else  /* !UNIV_HOTBACKUP */
  ut_a(sync);
  AIO_mode aio_mode = AIO_mode::SYNC;
#endif /* !UNIV_HOTBACKUP */

  /* Reserve the mutex and make sure that we can open at
  least one file while holding it, if the file is not already open */

  fil_space_t *space;

  bool slot = mutex_acquire_and_get_space(page_id.space(), space);

  /* If we are deleting a tablespace we don't allow async read
  operations on that. However, we do allow write operations and
  sync read operations. */
  if (space == nullptr ||
      (req_type.is_read() && !sync && space->stop_new_ops)) {
    if (slot) {
      release_open_slot(m_id);
    }

    mutex_release();

    if (!req_type.ignore_missing()) {
      if (space == nullptr) {
        ib::error(ER_IB_MSG_330)
            << "Trying to do I/O on a tablespace"
            << " which does not exist. I/O type: "
            << (req_type.is_read() ? "read" : "write") << ", page: " << page_id
            << ", I/O length: " << len << " bytes";
      } else {
        ib::error(ER_IB_MSG_331)
            << "Trying to do async read on a"
            << " tablespace which is being deleted."
            << " Tablespace name: \"" << space->name << "\", page: " << page_id
            << ", read length: " << len << " bytes";
      }
    }

    return (DB_TABLESPACE_DELETED);
  }

  ut_ad(aio_mode != AIO_mode::IBUF || fil_type_is_data(space->purpose));

  fil_node_t *file;
  page_no_t page_no = page_id.page_no();
  dberr_t err = get_file_for_io(req_type, space, &page_no, file);

  if (err == DB_TABLE_NOT_FOUND) {
    mutex_release();

    return (err);

  } else if (file == nullptr) {
    ut_ad(err == DB_ERROR);

    if (req_type.ignore_missing()) {
      if (slot) {
        release_open_slot(m_id);
      }

      mutex_release();

      return (DB_ERROR);
    }

    /* This is a hard error. */
    fil_report_invalid_page_access(page_id.page_no(), page_id.space(),
                                   space->name, byte_offset, len,
                                   req_type.is_read());
  }

  bool opened = prepare_file_for_io(file, false);

  if (slot) {
    release_open_slot(m_id);
  }

  if (!opened) {
    if (fil_type_is_data(space->purpose) && fsp_is_ibd_tablespace(space->id)) {
      mutex_release();

      if (!req_type.ignore_missing()) {
        ib::error(ER_IB_MSG_332)
            << "Trying to do I/O to a tablespace"
               " which exists without an .ibd data"
            << " file. I/O type: " << (req_type.is_read() ? "read" : "write")
            << ", page: " << page_id_t(page_id.space(), page_no)
            << ", I/O length: " << len << " bytes";
      }

      return (DB_TABLESPACE_DELETED);
    }

    /* The tablespace is for log. Currently, we just assert here
    to prevent handling errors along the way fil_io returns.
    Also, if the log files are missing, it would be hard to
    promise the server can continue running. */
    ut_a(0);
  }

  /* Check that at least the start offset is within the bounds of a
  single-table tablespace, including rollback tablespaces. */
  if (file->size <= page_no && space->id != TRX_SYS_SPACE &&
      fil_type_is_data(space->purpose)) {
    if (req_type.ignore_missing()) {
      /* If we can tolerate the non-existent pages, we
      should return with DB_ERROR and let caller decide
      what to do. */

      complete_io(file, req_type);

      mutex_release();

      return (DB_ERROR);
    }

    /* This is a hard error. */
    fil_report_invalid_page_access(page_id.page_no(), page_id.space(),
                                   space->name, byte_offset, len,
                                   req_type.is_read());
  }

  mutex_release();

  ut_a(page_size.is_compressed() ||
       page_size.physical() == page_size.logical());

  os_offset_t offset = (os_offset_t)page_no * page_size.physical();

  offset += byte_offset;

  ut_a(file->size - page_no >=
       (byte_offset + len + (page_size.physical() - 1)) / page_size.physical());

  ut_a((len % OS_FILE_LOG_BLOCK_SIZE) == 0);
  ut_a(byte_offset % OS_FILE_LOG_BLOCK_SIZE == 0);

  /* Don't compress the log, page 0 of all tablespaces, tables
  compresssed with the old compression scheme and all pages from
  the system tablespace. */

  if (req_type.is_write() && !req_type.is_log() && !page_size.is_compressed() &&
      page_id.page_no() > 0 && IORequest::is_punch_hole_supported() &&
      file->punch_hole) {
    ut_ad(!req_type.is_log());

    req_type.set_punch_hole();

    req_type.compression_algorithm(space->compression_type);

  } else {
    req_type.clear_compressed();
  }

  /* Set encryption information. */
  fil_io_set_encryption(req_type, page_id, space);

  req_type.block_size(file->block_size);

#ifdef UNIV_HOTBACKUP
  /* In mysqlbackup do normal I/O, not AIO */
  if (req_type.is_read()) {
    err = os_file_read(req_type, file->handle, buf, offset, len);

  } else {
    ut_ad(!srv_read_only_mode || fsp_is_system_temporary(page_id.space()));

    err = os_file_write(req_type, file->name, file->handle, buf, offset, len);
  }
#else /* UNIV_HOTBACKUP */
  /* Queue the aio request */
  err = os_aio(
      req_type, aio_mode, file->name, file->handle, buf, offset, len,
      fsp_is_system_temporary(page_id.space()) ? false : srv_read_only_mode,
      file, message);

#endif /* UNIV_HOTBACKUP */

  if (err == DB_IO_NO_PUNCH_HOLE) {
    err = DB_SUCCESS;

    if (file->punch_hole) {
      ib::warn(ER_IB_MSG_333) << "Punch hole failed for '" << file->name << "'";
    }

    fil_no_punch_hole(file);
  }

  /* We an try to recover the page from the double write buffer if
  the decompression fails or the page is corrupt. */

  ut_a(req_type.is_dblwr_recover() || err == DB_SUCCESS);

  if (sync) {
    /* The i/o operation is already completed when we return from
    os_aio: */

    mutex_acquire();

    complete_io(file, req_type);

    mutex_release();

    ut_ad(fil_validate_skip());
  }

  return (err);
}

#ifndef UNIV_HOTBACKUP


/** Waits for an AIO operation to complete. This function is used to write the
handler for completed requests. The aio array of pending requests is divided
into segments (see os0file.cc for more info). The thread specifies which
segment it wants to wait for.
@param[in]	segment		The number of the segment in the AIO array
                                to wait for */
void fil_aio_wait(ulint segment) {
  fil_node_t *file;
  IORequest type;
  void *message;

  ut_ad(fil_validate_skip());

  dberr_t err = os_aio_handler(segment, &file, &message, &type);

  ut_a(err == DB_SUCCESS);

  if (file == nullptr) {
    ut_ad(srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS);
    return;
  }

  srv_set_io_thread_op_info(segment, "complete io for file");

  auto shard = fil_system->shard_by_id(file->space->id);

  shard->mutex_acquire();

  shard->complete_io(file, type);

  shard->mutex_release();

  ut_ad(fil_validate_skip());

  /* Do the i/o handling */
  /* IMPORTANT: since i/o handling for reads will read also the insert
  buffer in tablespace 0, you have to be very careful not to introduce
  deadlocks in the i/o system. We keep tablespace 0 data files always
  open, and use a special i/o thread to serve insert buffer requests. */

  switch (file->space->purpose) {
    case FIL_TYPE_IMPORT:
    case FIL_TYPE_TEMPORARY:
    case FIL_TYPE_TABLESPACE:
      srv_set_io_thread_op_info(segment, "complete io for buf page");

      /* async single page writes from the dblwr buffer don't have
      access to the page */
      if (message != nullptr) {
        buf_page_io_complete(static_cast<buf_page_t *>(message));
      }
      return;
    case FIL_TYPE_LOG:
      return;
  }

  ut_ad(0);
}
#endif /* !UNIV_HOTBACKUP */





/** Flushes to disk possible writes cached by the OS. If the space does
not exist or is being dropped, does not do anything.
@param[in]	space_id	File space ID (this can be a group of log files
                                or a tablespace of the database) */
void Fil_shard::space_flush(space_id_t space_id) {
  ut_ad(mutex_owned());

  if (space_id == dict_sys_t_s_log_space_first_id) {
    redo_space_flush();
    return;
  }

  fil_space_t *space = get_space_by_id(space_id);

  if (space == nullptr || space->purpose == FIL_TYPE_TEMPORARY ||
      space->stop_new_ops) {
    return;
  }

  bool fbd = fil_buffering_disabled(space);

  if (fbd) {
    /* No need to flush. User has explicitly disabled
    buffering. However, flush should be called if the file
    size changes to keep OЅ metadata in sync. */
    ut_ad(!space->is_in_unflushed_spaces);
    ut_ad(space_is_flushed(space));

    /* Flush only if the file size changes */
    bool no_flush = true;
    for (const auto &file : space->files) {
#ifdef UNIV_DEBUG
      ut_ad(file.modification_counter == file.flush_counter);
#endif /* UNIV_DEBUG */
      if (file.flush_size != file.size) {
        /* Found at least one file whose size has changed */
        no_flush = false;
        break;
      }
    }

    if (no_flush) {
      /* Nothing to flush. Just return */
      return;
    }
  }

  /* Prevent dropping of the space while we are flushing */
  ++space->n_pending_flushes;

  for (auto &file : space->files) {
    int64_t old_mod_counter = file.modification_counter;

    if (!file.is_open) {
      continue;
    }

    /* Skip flushing if the file size has not changed since
    last flush was done and the flush mode is O_DIRECT_NO_FSYNC */
    if (fbd && (file.flush_size == file.size)) {
      ut_ad(old_mod_counter <= file.flush_counter);
      continue;
    }

    /* If we are here and the flush mode is O_DIRECT_NO_FSYNC, then
    it means that the file size has changed and hence, it should be
    flushed, irrespective of the mod_counter and flush counter values,
    which are always same in case of O_DIRECT_NO_FSYNC to avoid flush
    on every write operation.
    For other flush modes, if the flush_counter is same or ahead of
    the mod_counter, skip the flush. */
    if (!fbd && (old_mod_counter <= file.flush_counter)) {
      continue;
    }

    switch (space->purpose) {
      case FIL_TYPE_TEMPORARY:
        ut_ad(0);  // we already checked for this

      case FIL_TYPE_TABLESPACE:
      case FIL_TYPE_IMPORT:
        ++fil_n_pending_tablespace_flushes;
        break;

      case FIL_TYPE_LOG:
        ut_error;
        break;
    }

    bool skip_flush = false;
#ifdef _WIN32
    if (file.is_raw_disk) {
      skip_flush = true;
    }
#endif /* _WIN32 */

    while (file.n_pending_flushes > 0 && !skip_flush) {
      /* We want to avoid calling os_file_flush() on
      the file twice at the same time, because we do
      not know what bugs OS's may contain in file
      I/O */

      int64_t sig_count = os_event_reset(file.sync_event);

      mutex_release();

      os_event_wait_low(file.sync_event, sig_count);

      mutex_acquire();

      if (file.flush_counter >= old_mod_counter) {
        skip_flush = true;
      }
    }

    if (!skip_flush) {
      ut_a(file.is_open);

      ++file.n_pending_flushes;

      mutex_release();

      os_file_flush(file.handle);

      file.flush_size = file.size;

      mutex_acquire();

      os_event_set(file.sync_event);

      --file.n_pending_flushes;
    }

    if (file.flush_counter < old_mod_counter) {
      file.flush_counter = old_mod_counter;

      remove_from_unflushed_list(space);
    }

    switch (space->purpose) {
      case FIL_TYPE_TEMPORARY:
        ut_ad(0);  // we already checked for this

      case FIL_TYPE_TABLESPACE:
      case FIL_TYPE_IMPORT:
        --fil_n_pending_tablespace_flushes;
        continue;

      case FIL_TYPE_LOG:
        ut_error;
    }

    ut_ad(0);
  }

  --space->n_pending_flushes;
}


/** Flush any pending writes to disk for the redo log. */
void Fil_shard::flush_file_redo() {
  /* We never evict the redo log tablespace. It's for all
  practical purposes a read-only data structure. */

  mutex_acquire();

  redo_space_flush();

  mutex_release();
}

/** Collect the tablespace IDs of unflushed tablespaces in space_ids.
@param[in]	purpose		FIL_TYPE_TABLESPACE or FIL_TYPE_LOG,
                                can be ORred */
void Fil_shard::flush_file_spaces(uint8_t purpose) {
  Space_ids space_ids;

  ut_ad((purpose & FIL_TYPE_TABLESPACE) || (purpose & FIL_TYPE_LOG));

  mutex_acquire();

  for (auto space = UT_LIST_GET_FIRST(m_unflushed_spaces); space != nullptr;
       space = UT_LIST_GET_NEXT(unflushed_spaces, space)) {
    if ((to_int(space->purpose) & purpose) && !space->stop_new_ops) {
      space_ids.push_back(space->id);
    }
  }

  mutex_release();

  /* Flush the spaces.  It will not hurt to call fil_flush() on
  a non-existing space id. */
  for (auto space_id : space_ids) {
    mutex_acquire();

    space_flush(space_id);

    mutex_release();
  }
}










/** Reset the page type.
Data files created before MySQL 5.1 may contain garbage in FIL_PAGE_TYPE.
In MySQL 3.23.53, only undo log pages and index pages were tagged.
Any other pages were written with uninitialized bytes in FIL_PAGE_TYPE.
@param[in]	page_id	page number
@param[in,out]	page	page with invalid FIL_PAGE_TYPE
@param[in]	type	expected page type
@param[in,out]	mtr	mini-transaction */
void fil_page_reset_type(const page_id_t &page_id, byte *page, ulint type,
                         mtr_t *mtr) {
  ib::info(ER_IB_MSG_334) << "Resetting invalid page " << page_id << " type "
                          << fil_page_get_type(page) << " to " << type << ".";
  mlog_write_ulint(page + FIL_PAGE_TYPE, type, MLOG_2BYTES, mtr);
}



#ifndef UNIV_HOTBACKUP
/** Initializes a buffer control block when the buf_pool is created.
@param[in]	block		Pointer to the control block
@param[in]	frame		Pointer to buffer frame */
static void fil_buf_block_init(buf_block_t *block, byte *frame) {
  UNIV_MEM_DESC(frame, UNIV_PAGE_SIZE);

  block->frame = frame;

  block->page.io_fix = BUF_IO_NONE;
  /* There are assertions that check for this. */
  block->page.buf_fix_count = 1;
  block->page.state = BUF_BLOCK_READY_FOR_USE;

  page_zip_des_init(&block->page.zip);
}

struct Fil_page_iterator {
  /** File handle */
  pfs_os_file_t m_file;

  /** File path name */
  const char *m_filepath;

  /** From where to start */
  os_offset_t m_start;

  /** Where to stop */
  os_offset_t m_end;

  /* File size in bytes */
  os_offset_t m_file_size;

  /** Page size */
  size_t m_page_size;

  /** Number of pages to use for I/O */
  size_t m_n_io_buffers;

  /** Buffer to use for IO */
  byte *m_io_buffer;

  /** Encryption key */
  byte *m_encryption_key;

  /** Encruption iv */
  byte *m_encryption_iv;
};

/** TODO: This can be made parallel trivially by chunking up the file
and creating a callback per thread. Main benefit will be to use multiple
CPUs for checksums and compressed tables. We have to do compressed tables
block by block right now. Secondly we need to decompress/compress and copy
too much of data. These are CPU intensive.

Iterate over all the pages in the tablespace.
@param[in]	iter		Tablespace iterator
@param[in,out]	block		Block to use for IO
@param[in]	callback	Callback to inspect and update page contents
@retval DB_SUCCESS or error code */
static dberr_t fil_iterate(const Fil_page_iterator &iter, buf_block_t *block,
                           PageCallback &callback) {
  os_offset_t offset;
  size_t n_bytes;
  page_no_t page_no = 0;
  space_id_t space_id = callback.get_space_id();

  n_bytes = iter.m_n_io_buffers * iter.m_page_size;

  ut_ad(!srv_read_only_mode);

  /* For old style compressed tables we do a lot of useless copying
  for non-index pages. Unfortunately, it is required by
  buf_zip_decompress() */

  ulint read_type = IORequest::READ;
  ulint write_type = IORequest::WRITE;

  for (offset = iter.m_start; offset < iter.m_end; offset += n_bytes) {
    byte *io_buffer = iter.m_io_buffer;

    block->frame = io_buffer;

    if (callback.get_page_size().is_compressed()) {
      page_zip_des_init(&block->page.zip);
      page_zip_set_size(&block->page.zip, iter.m_page_size);

      block->page.size.copy_from(
          page_size_t(iter.m_page_size, univ_page_size.logical(), true));

      block->page.zip.data = block->frame + UNIV_PAGE_SIZE;
      ut_d(block->page.zip.m_external = true);
      ut_ad(iter.m_page_size == callback.get_page_size().physical());

      /* Zip IO is done in the compressed page buffer. */
      io_buffer = block->page.zip.data;
    } else {
      io_buffer = iter.m_io_buffer;
    }

    /* We have to read the exact number of bytes. Otherwise the
    InnoDB IO functions croak on failed reads. */

    n_bytes = static_cast<ulint>(
        std::min(static_cast<os_offset_t>(n_bytes), iter.m_end - offset));

    ut_ad(n_bytes > 0);
    ut_ad(!(n_bytes % iter.m_page_size));

    dberr_t err;
    IORequest read_request(read_type);

    /* For encrypted table, set encryption information. */
    if (iter.m_encryption_key != nullptr && offset != 0) {
      read_request.encryption_key(iter.m_encryption_key, ENCRYPTION_KEY_LEN,
                                  iter.m_encryption_iv);

      read_request.encryption_algorithm(Encryption::AES);
    }

    err = os_file_read(read_request, iter.m_file, io_buffer, offset,
                       (ulint)n_bytes);

    if (err != DB_SUCCESS) {
      ib::error(ER_IB_MSG_335) << "os_file_read() failed";

      return (err);
    }

    size_t n_pages_read;
    bool updated = false;
    os_offset_t page_off = offset;

    n_pages_read = (ulint)n_bytes / iter.m_page_size;

    for (size_t i = 0; i < n_pages_read; ++i) {
      buf_block_set_file_page(block, page_id_t(space_id, page_no++));

      if ((err = callback(page_off, block)) != DB_SUCCESS) {
        return (err);

      } else if (!updated) {
        updated = buf_block_get_state(block) == BUF_BLOCK_FILE_PAGE;
      }

      buf_block_set_state(block, BUF_BLOCK_NOT_USED);
      buf_block_set_state(block, BUF_BLOCK_READY_FOR_USE);

      page_off += iter.m_page_size;
      block->frame += iter.m_page_size;
    }

    IORequest write_request(write_type);

    /* For encrypted table, set encryption information. */
    if (iter.m_encryption_key != nullptr && offset != 0) {
      write_request.encryption_key(iter.m_encryption_key, ENCRYPTION_KEY_LEN,
                                   iter.m_encryption_iv);

      write_request.encryption_algorithm(Encryption::AES);
    }

    /* A page was updated in the set, write back to disk.
    Note: We don't have the compression algorithm, we write
    out the imported file as uncompressed. */

    if (updated && (err = os_file_write(write_request, iter.m_filepath,
                                        iter.m_file, io_buffer, offset,
                                        (ulint)n_bytes)) != DB_SUCCESS) {
      /* This is not a hard error */
      if (err == DB_IO_NO_PUNCH_HOLE) {
        err = DB_SUCCESS;
        write_type &= ~IORequest::PUNCH_HOLE;

      } else {
        ib::error(ER_IB_MSG_336) << "os_file_write() failed";

        return (err);
      }
    }
  }

  return (DB_SUCCESS);
}

/** Iterate over all the pages in the tablespace.
@param[in,out]	table		the table definiton in the server
@param[in]	n_io_buffers	number of blocks to read and write together
@param[in]	callback	functor that will do the page updates
@return DB_SUCCESS or error code */
dberr_t fil_tablespace_iterate(dict_table_t *table, ulint n_io_buffers,
                               PageCallback &callback) {
  dberr_t err;
  pfs_os_file_t file;
  char *filepath;
  bool success;

  ut_a(n_io_buffers > 0);
  ut_ad(!srv_read_only_mode);

  DBUG_EXECUTE_IF("ib_import_trigger_corruption_1", return (DB_CORRUPTION););

  /* Make sure the data_dir_path is set. */
  dd_get_and_save_data_dir_path<dd::Table>(table, nullptr, false);

  std::string path = dict_table_get_datadir(table);

  filepath = Fil_path::make(path, table->name.m_name, IBD, true);

  if (filepath == nullptr) {
    return (DB_OUT_OF_MEMORY);
  }

  file = os_file_create_simple_no_error_handling(
      innodb_data_file_key, filepath, OS_FILE_OPEN, OS_FILE_READ_WRITE,
      srv_read_only_mode, &success);

  DBUG_EXECUTE_IF("fil_tablespace_iterate_failure", {
    static bool once;

    if (!once || ut_rnd_interval(0, 10) == 5) {
      once = true;
      success = false;
      os_file_close(file);
    }
  });

  if (!success) {
    /* The following call prints an error message */
    os_file_get_last_error(true);

    ib::error(ER_IB_MSG_337) << "Trying to import a tablespace, but could not"
                                " open the tablespace file "
                             << filepath;

    ut_free(filepath);

    return (DB_TABLESPACE_NOT_FOUND);

  } else {
    err = DB_SUCCESS;
  }

  callback.set_file(filepath, file);

  os_offset_t file_size = os_file_get_size(file);
  ut_a(file_size != (os_offset_t)-1);

  /* The block we will use for every physical page */
  buf_block_t *block;

  block = reinterpret_cast<buf_block_t *>(ut_zalloc_nokey(sizeof(*block)));

  mutex_create(LATCH_ID_BUF_BLOCK_MUTEX, &block->mutex);

  /* Allocate a page to read in the tablespace header, so that we
  can determine the page size and zip size (if it is compressed).
  We allocate an extra page in case it is a compressed table. One
  page is to ensure alignement. */

  void *page_ptr = ut_malloc_nokey(3 * UNIV_PAGE_SIZE);
  byte *page = static_cast<byte *>(ut_align(page_ptr, UNIV_PAGE_SIZE));

  fil_buf_block_init(block, page);

  /* Read the first page and determine the page and zip size. */

  IORequest request(IORequest::READ);

  err = os_file_read_first_page(request, file, page, UNIV_PAGE_SIZE);

  if (err != DB_SUCCESS) {
    err = DB_IO_ERROR;

  } else if ((err = callback.init(file_size, block)) == DB_SUCCESS) {
    Fil_page_iterator iter;

    iter.m_file = file;
    iter.m_start = 0;
    iter.m_end = file_size;
    iter.m_filepath = filepath;
    iter.m_file_size = file_size;
    iter.m_n_io_buffers = n_io_buffers;
    iter.m_page_size = callback.get_page_size().physical();

    /* Set encryption info. */
    iter.m_encryption_key = table->encryption_key;
    iter.m_encryption_iv = table->encryption_iv;

    /* Check encryption is matched or not. */
    ulint space_flags = callback.get_space_flags();

    if (FSP_FLAGS_GET_ENCRYPTION(space_flags)) {
      ut_ad(table->encryption_key != nullptr);

      if (!dd_is_table_in_encrypted_tablespace(table)) {
        ib::error(ER_IB_MSG_338) << "Table is not in an encrypted"
                                    " tablespace, but the data file which"
                                    " trying to import is an encrypted"
                                    " tablespace";

        err = DB_IO_NO_ENCRYPT_TABLESPACE;
      }
    }

    if (err == DB_SUCCESS) {
      /* Compressed pages can't be optimised for block IO
      for now.  We do the IMPORT page by page. */

      if (callback.get_page_size().is_compressed()) {
        iter.m_n_io_buffers = 1;
        ut_a(iter.m_page_size == callback.get_page_size().physical());
      }

      /** Add an extra page for compressed page scratch
      area. */
      void *io_buffer =
          ut_malloc_nokey((2 + iter.m_n_io_buffers) * UNIV_PAGE_SIZE);

      iter.m_io_buffer =
          static_cast<byte *>(ut_align(io_buffer, UNIV_PAGE_SIZE));

      err = fil_iterate(iter, block, callback);

      ut_free(io_buffer);
    }
  }

  if (err == DB_SUCCESS) {
    ib::info(ER_IB_MSG_339) << "Sync to disk";

    if (!os_file_flush(file)) {
      ib::info(ER_IB_MSG_340) << "os_file_flush() failed!";
      err = DB_IO_ERROR;
    } else {
      ib::info(ER_IB_MSG_341) << "Sync to disk - done!";
    }
  }

  os_file_close(file);

  ut_free(page_ptr);
  ut_free(filepath);

  mutex_free(&block->mutex);

  ut_free(block);

  return (err);
}
#endif /* !UNIV_HOTBACKUP */



#ifndef UNIV_HOTBACKUP
/** Check if swapping two .ibd files can be done without failure.
@param[in]	old_table	old table
@param[in]	new_table	new table
@param[in]	tmp_name	temporary table name
@return innodb error code */
dberr_t fil_rename_precheck(const dict_table_t *old_table,
                            const dict_table_t *new_table,
                            const char *tmp_name) {
  dberr_t err;

  bool old_is_file_per_table = dict_table_is_file_per_table(old_table);

  bool new_is_file_per_table = dict_table_is_file_per_table(new_table);

  /* If neither table is file-per-table,
  there will be no renaming of files. */
  if (!old_is_file_per_table && !new_is_file_per_table) {
    return (DB_SUCCESS);
  }

  auto old_dir = dict_table_get_datadir(old_table);

  char *old_path =
      Fil_path::make(old_dir, old_table->name.m_name, IBD, !old_dir.empty());

  if (old_path == nullptr) {
    return (DB_OUT_OF_MEMORY);
  }

  if (old_is_file_per_table) {
    char *tmp_path = Fil_path::make(old_dir, tmp_name, IBD, !old_dir.empty());

    if (tmp_path == nullptr) {
      ut_free(old_path);
      return (DB_OUT_OF_MEMORY);
    }

    /* Temp filepath must not exist. */
    err = fil_rename_tablespace_check(old_table->space, old_path, tmp_path,
                                      dict_table_is_discarded(old_table));

    if (err != DB_SUCCESS) {
      ut_free(old_path);
      ut_free(tmp_path);
      return (err);
    }

    ut_free(tmp_path);
  }

  if (new_is_file_per_table) {
    auto new_dir = dict_table_get_datadir(new_table);

    char *new_path =
        Fil_path::make(new_dir, new_table->name.m_name, IBD, !new_dir.empty());

    if (new_path == nullptr) {
      ut_free(old_path);
      return (DB_OUT_OF_MEMORY);
    }

    /* Destination filepath must not exist unless this ALTER
    TABLE starts and ends with a file_per-table tablespace. */
    if (!old_is_file_per_table) {
      err = fil_rename_tablespace_check(new_table->space, new_path, old_path,
                                        dict_table_is_discarded(new_table));

      if (err != DB_SUCCESS) {
        ut_free(old_path);
        ut_free(new_path);
        return (err);
      }
    }

    ut_free(new_path);
  }

  ut_free(old_path);

  return (DB_SUCCESS);
}
#endif /* !UNIV_HOTBACKUP */



/** Set the compression type for the tablespace of a table
@param[in]	table		The table that should be compressed
@param[in]	algorithm	Text representation of the algorithm
@return DB_SUCCESS or error code */
dberr_t fil_set_compression(dict_table_t *table, const char *algorithm) {
  ut_ad(table != nullptr);

  /* We don't support Page Compression for the system tablespace,
  the temporary tablespace, or any general tablespace because
  COMPRESSION is set by TABLE DDL, not TABLESPACE DDL. There is
  no other technical reason.  Also, do not use it for missing
  tables or tables with compressed row_format. */
  if (table->ibd_file_missing ||
      !DICT_TF2_FLAG_IS_SET(table, DICT_TF2_USE_FILE_PER_TABLE) ||
      DICT_TF2_FLAG_IS_SET(table, DICT_TF2_TEMPORARY) ||
      page_size_t(table->flags).is_compressed()) {
    return (DB_IO_NO_PUNCH_HOLE_TABLESPACE);
  }

  dberr_t err;
  Compression compression;

  if (algorithm == nullptr || strlen(algorithm) == 0) {
#ifndef UNIV_DEBUG
    compression.m_type = Compression::NONE;
#else /* UNIV_DEBUG */
    /* This is a Debug tool for setting compression on all
    compressible tables not otherwise specified. */
    switch (srv_debug_compress) {
      case Compression::LZ4:
      case Compression::ZLIB:
      case Compression::NONE:

        compression.m_type = static_cast<Compression::Type>(srv_debug_compress);
        break;

      default:
        compression.m_type = Compression::NONE;
    }

#endif /* UNIV_DEBUG */

    err = DB_SUCCESS;

  } else {
    err = Compression::check(algorithm, &compression);
  }

  fil_space_t *space = fil_space_get(table->space);

  if (space == nullptr) {
    return (DB_NOT_FOUND);
  }

  space->compression_type = compression.m_type;

  if (space->compression_type != Compression::NONE) {
    if (!space->files.front().punch_hole) {
      return (DB_IO_NO_PUNCH_HOLE_FS);
    }
  }

  return (err);
}




#ifndef UNIV_HOTBACKUP
/** Rotate the tablespace keys by new master key.
@param[in,out]	shard		Rotate the keys in this shard
@return true if the re-encrypt succeeds */
bool Fil_system::encryption_rotate_in_a_shard(Fil_shard *shard) {
  byte encrypt_info[ENCRYPTION_INFO_SIZE];

  for (auto &elem : shard->m_spaces) {
    auto space = elem.second;

    /* Skip unencypted tablespaces. Encrypted redo log
    tablespaces is handled in function log_rotate_encryption. */

    if (fsp_is_system_or_temp_tablespace(space->id) ||
        space->purpose == FIL_TYPE_LOG) {
      continue;
    }

    /* Skip the undo tablespace when it's in default key status,
    since it's the first server startup after bootstrap, and the
    server uuid is not ready yet. */

    if (fsp_is_undo_tablespace(space->id) &&
        Encryption::s_master_key_id == ENCRYPTION_DEFAULT_MASTER_KEY_ID) {
      continue;
    }

    /* Rotate the encrypted tablespaces. */
    if (space->encryption_type != Encryption::NONE) {
      memset(encrypt_info, 0, ENCRYPTION_INFO_SIZE);

      /* Take MDL on UNDO tablespace to make it mutually exclusive with
      UNDO tablespace truncation. For other tablespaces MDL is not required
      here. */
      MDL_ticket *mdl_ticket = nullptr;
      if (fsp_is_undo_tablespace(space->id)) {
        THD *thd = current_thd;
        while (
            acquire_shared_backup_lock(thd, thd->variables.lock_wait_timeout)) {
          os_thread_sleep(20);
        }

        while (dd::acquire_exclusive_tablespace_mdl(thd, space->name, false,
                                                    &mdl_ticket, false)) {
          os_thread_sleep(20);
        }
        ut_ad(mdl_ticket != nullptr);
      }

      mtr_t mtr;
      mtr_start(&mtr);
      bool ret = fsp_header_rotate_encryption(space, encrypt_info, &mtr);
      mtr_commit(&mtr);

      if (mdl_ticket != nullptr) {
        dd_release_mdl(mdl_ticket);
      }
      if (!ret) {
        return (false);
      }
    }

    DBUG_EXECUTE_IF("ib_crash_during_rotation_for_encryption", DBUG_SUICIDE(););
  }

  return (true);
}





#endif /* !UNIV_HOTBACKUP */






/* Unit Tests */
#ifdef UNIV_ENABLE_UNIT_TEST_MAKE_FILEPATH
#define MF Fil_path::make
#define DISPLAY ib::info(ER_IB_MSG_342) << path
void test_make_filepath() {
  char *path;
  const char *long_path =
      "this/is/a/very/long/path/including/a/very/"
      "looooooooooooooooooooooooooooooooooooooooooooooooo"
      "oooooooooooooooooooooooooooooooooooooooooooooooooo"
      "oooooooooooooooooooooooooooooooooooooooooooooooooo"
      "oooooooooooooooooooooooooooooooooooooooooooooooooo"
      "oooooooooooooooooooooooooooooooooooooooooooooooooo"
      "oooooooooooooooooooooooooooooooooooooooooooooooooo"
      "oooooooooooooooooooooooooooooooooooooooooooooooooo"
      "oooooooooooooooooooooooooooooooooooooooooooooooooo"
      "oooooooooooooooooooooooooooooooooooooooooooooooooo"
      "oooooooooooooooooooooooooooooooooooooooooooooooong"
      "/folder/name";
  path = MF("/this/is/a/path/with/a/filename", nullptr, IBD, false);
  DISPLAY;
  path = MF("/this/is/a/path/with/a/filename", nullptr, ISL, false);
  DISPLAY;
  path = MF("/this/is/a/path/with/a/filename", nullptr, CFG, false);
  DISPLAY;
  path = MF("/this/is/a/path/with/a/filename", nullptr, CFP, false);
  DISPLAY;
  path = MF("/this/is/a/path/with/a/filename.ibd", nullptr, IBD, false);
  DISPLAY;
  path = MF("/this/is/a/path/with/a/filename.ibd", nullptr, IBD, false);
  DISPLAY;
  path = MF("/this/is/a/path/with/a/filename.dat", nullptr, IBD, false);
  DISPLAY;
  path = MF(nullptr, "tablespacename", NO_EXT, false);
  DISPLAY;
  path = MF(nullptr, "tablespacename", IBD, false);
  DISPLAY;
  path = MF(nullptr, "dbname/tablespacename", NO_EXT, false);
  DISPLAY;
  path = MF(nullptr, "dbname/tablespacename", IBD, false);
  DISPLAY;
  path = MF(nullptr, "dbname/tablespacename", ISL, false);
  DISPLAY;
  path = MF(nullptr, "dbname/tablespacename", CFG, false);
  DISPLAY;
  path = MF(nullptr, "dbname/tablespacename", CFP, false);
  DISPLAY;
  path = MF(nullptr, "dbname\\tablespacename", NO_EXT, false);
  DISPLAY;
  path = MF(nullptr, "dbname\\tablespacename", IBD, false);
  DISPLAY;
  path = MF("/this/is/a/path", "dbname/tablespacename", IBD, false);
  DISPLAY;
  path = MF("/this/is/a/path", "dbname/tablespacename", IBD, true);
  DISPLAY;
  path = MF("./this/is/a/path", "dbname/tablespacename.ibd", IBD, true);
  DISPLAY;
  path = MF("this\\is\\a\\path", "dbname/tablespacename", IBD, true);
  DISPLAY;
  path = MF("/this/is/a/path", "dbname\\tablespacename", IBD, true);
  DISPLAY;
  path = MF(long_path, nullptr, IBD, false);
  DISPLAY;
  path = MF(long_path, "tablespacename", IBD, false);
  DISPLAY;
  path = MF(long_path, "tablespacename", IBD, true);
  DISPLAY;
}
#endif /* UNIV_ENABLE_UNIT_TEST_MAKE_FILEPATH */



#ifndef UNIV_HOTBACKUP


/** Initialize the table space encryption
@param[in,out]	space		Tablespace instance */
static void fil_tablespace_encryption_init(const fil_space_t *space) {
  for (auto &key : *recv_sys->keys) {
    if (key.space_id != space->id) {
      continue;
    }

    dberr_t err = DB_SUCCESS;

    ut_ad(!fsp_is_system_tablespace(space->id));

    /* Here we try to populate space tablespace_key which is read during
    REDO scan.

    Consider following scenario:
    1. Alter tablespce .. encrypt=y (KEY1)
    2. Alter tablespce .. encrypt=n
    3. Alter tablespce .. encrypt=y (KEY2)

    Lets say there is a crash after (3) is finished successfully. All the pages
    of tablespace are encrypted with KEY2.

    During recovery:
    ----------------
    - Let's say we scanned till REDO of (1) but couldn't reach to REDO of (3).
    - So we've got tablespace key as KEY1.
    - Note, tablespace pages were encrypted using KEY2 which would have been
      found on page 0 and thus loaded already in file_space_t.

    If we overwrite this space key (KEY2) with the one we got from REDO log
    scan (KEY1), then when we try to read a page from Disk, we will try to
    decrypt it using KEY1 whereas page was encrypted with KEY2. ERROR.

    Therefore, for a general tablespace, if tablespace key is already populated
    it is the latest key and should be used instead of the one read during
    REDO log scan.

    For file-per-table tablespace, which is not INPLACE algorithm, copy what
    is found on REDO Log.
    */
    if (fsp_is_file_per_table(space->id, space->flags) ||
        space->encryption_klen == 0) {
      err = fil_set_encryption(space->id, Encryption::AES, key.ptr, key.iv);
    }

    if (err != DB_SUCCESS) {
      ib::error(ER_IB_MSG_343) << "Can't set encryption information"
                               << " for tablespace" << space->name << "!";
    }

    ut_free(key.iv);
    ut_free(key.ptr);

    key.iv = nullptr;
    key.ptr = nullptr;

    key.space_id = std::numeric_limits<space_id_t>::max();
  }
}

/** Update the DD if any files were moved to a new location.
Free the Tablespace_files instance.
@param[in]	read_only_mode	true if InnoDB is started in read only mode.
@return DB_SUCCESS if all OK */
dberr_t Fil_system::prepare_open_for_business(bool read_only_mode) {
  if (read_only_mode && !m_moved.empty()) {
    ib::error(ER_IB_MSG_344)
        << m_moved.size() << " files have been relocated"
        << " and the server has been started in read"
        << " only mode. Cannot update the data dictionary.";

    return (DB_READ_ONLY);
  }

  trx_t *trx = check_trx_exists(current_thd);

  TrxInInnoDB trx_in_innodb(trx);

  /* The transaction should not be active yet, start it */

  trx->isolation_level = trx_t::READ_UNCOMMITTED;

  trx_start_if_not_started_xa(trx, false);

  size_t count = 0;
  size_t failed = 0;
  size_t batch_size = 0;
  bool print_msg = false;
  auto start_time = ut_time();

  /* If some file paths have changed then update the DD */
  for (auto &tablespace : m_moved) {
    dberr_t err;

    auto old_path = std::get<dd_fil::OLD_PATH>(tablespace);

    auto space_name = std::get<dd_fil::SPACE_NAME>(tablespace);

    auto new_path = std::get<dd_fil::NEW_PATH>(tablespace);
    auto object_id = std::get<dd_fil::OBJECT_ID>(tablespace);

    err = dd_rename_tablespace(object_id, space_name.c_str(), new_path.c_str());

    if (err != DB_SUCCESS) {
      ib::error(ER_IB_MSG_345) << "Unable to update tablespace ID"
                               << " " << object_id << " "
                               << " '" << old_path << "' to"
                               << " '" << new_path << "'";

      ++failed;
    }

    ++count;

    if (ut_time() - start_time >= PRINT_INTERVAL_SECS) {
      ib::info(ER_IB_MSG_346) << "Processed " << count << "/" << m_moved.size()
                              << " tablespace paths. Failures " << failed;

      start_time = ut_time();
      print_msg = true;
    }

    ++batch_size;

    if (batch_size > 10000) {
      innobase_commit_low(trx);

      ib::info(ER_IB_MSG_347) << "Committed : " << batch_size;

      batch_size = 0;

      trx_start_if_not_started_xa(trx, false);
    }
  }

  if (batch_size > 0) {
    ib::info(ER_IB_MSG_348) << "Committed : " << batch_size;
  }

  innobase_commit_low(trx);

  if (print_msg) {
    ib::info(ER_IB_MSG_349) << "Updated " << count << " tablespace paths"
                            << ", failures " << failed;
  }

  return (failed == 0 ? DB_SUCCESS : DB_ERROR);
}



/** Replay a file rename operation for ddl replay.
@param[in]	page_id		Space ID and first page number in the file
@param[in]	old_name	old file name
@param[in]	new_name	new file name
@return	whether the operation was successfully applied (the name did not
exist, or new_name did not exist and name was successfully renamed to
new_name)  */
bool fil_op_replay_rename_for_ddl(const page_id_t &page_id,
                                  const char *old_name, const char *new_name) {
  space_id_t space_id = page_id.space();
  fil_space_t *space = fil_space_get(space_id);

  if (space == nullptr && !fil_system->open_for_recovery(space_id)) {
    ib::info(ER_IB_MSG_350)
        << "Can not find space with space ID " << space_id
        << " when replaying the DDL log "
        << "rename from '" << old_name << "' to '" << new_name << "'";

    return (true);
  }

  return (fil_op_replay_rename(page_id, old_name, new_name));
}

/** Lookup the tablespace ID for recovery and DDL log apply.
@param[in]	space_id		Tablespace ID to lookup
@return true if the space ID is known. */
bool Fil_system::lookup_for_recovery(space_id_t space_id) {
  ut_ad(recv_recovery_is_on() || Log_DDL::is_in_recovery());

  /* Single threaded code, no need to acquire mutex. */
  const auto result = get_scanned_files(space_id);

  if (recv_recovery_is_on()) {
    const auto &end = recv_sys->deleted.end();
    const auto &it = recv_sys->deleted.find(space_id);

    if (result.second == nullptr) {
      /* If it wasn't deleted after finding it on disk then
      we tag it as missing. */

      if (it == end) {
        recv_sys->missing_ids.insert(space_id);
      }

      return (false);
    }

    /* Check that it wasn't deleted. */

    return (it == end);
  }

  return (result.second != nullptr);
}



/** Open a tablespace that has a redo/DDL log record to apply.
@param[in]	space_id		Tablespace ID
@return true if the open was successful */
bool Fil_system::open_for_recovery(space_id_t space_id) {
  ut_ad(recv_recovery_is_on() || Log_DDL::is_in_recovery());

  if (!lookup_for_recovery(space_id)) {
    return (false);
  }

  const auto result = get_scanned_files(space_id);

  /* Duplicates should have been sorted out before start of recovery. */
  ut_a(result.second->size() == 1);

  const auto &filename = result.second->front();
  const std::string path = result.first + filename;

  fil_space_t *space;

  auto status = ibd_open_for_recovery(space_id, path, space);

  if (status == FIL_LOAD_OK) {
    /* For encrypted tablespace, set key and iv. */
    if (FSP_FLAGS_GET_ENCRYPTION(space->flags) && recv_sys->keys != nullptr) {
      fil_tablespace_encryption_init(space);
    }

    if (!recv_sys->dblwr.deferred.empty()) {
      buf_dblwr_recover_pages(space);
    }

    return (true);
  }

  return (false);
}



/** Lookup the tablespace ID and return the path to the file. The filename
is ignored when testing for equality. Only the path up to the file name is
considered for matching: e.g. ./test/a.ibd == ./test/b.ibd.
@param[in]	dd_object_id	Server DD tablespace ID
@param[in]	space_id	Tablespace ID to lookup
@param[in]	space_name	Tablespace name
@param[in]	old_path	Path in the data dictionary
@param[out]	new_path	New path if scanned path not equal to path
@return status of the match. */
Fil_state fil_tablespace_path_equals(dd::Object_id dd_object_id,
                                     space_id_t space_id,
                                     const char *space_name,
                                     std::string old_path,
                                     std::string *new_path) {
  ut_ad(Fil_path::has_suffix(IBD, old_path));

  /* Single threaded code, no need to acquire mutex. */
  const auto &end = recv_sys->deleted.end();
  const auto result = fil_system->get_scanned_files(space_id);
  const auto &it = recv_sys->deleted.find(space_id);

  if (result.second == nullptr) {
    /* If the DD has the path but --innodb-directories doesn't,
    we need to check if the DD path is valid before we tag the
    file as missing. */

    if (Fil_path::get_file_type(old_path) == OS_FILE_TYPE_FILE) {
      ib::info(ER_IB_MSG_352) << old_path << " found outside of"
                              << " --innodb-directories setting";

      return (Fil_state::MATCHES);
    }

    /* If it wasn't deleted during redo apply, we tag it
    as missing. */

    if (it == end && recv_recovery_is_on()) {
      recv_sys->missing_ids.insert(space_id);
    }

    return (Fil_state::MISSING);
  }

  /* Check that it wasn't deleted. */
  if (it != end) {
    return (Fil_state::DELETED);
  }

  /* A file with this space_id was found during scanning.
  Validate its location and see if it was moved.

  Don't compare the full filename, there can be a mismatch if
  there was a DDL in progress and we will end up renaming the path
  in the DD dictionary. Such renames should be handled by the
  atomic DDL "ddl_log". */

  std::string old_dir{old_path};

  /* Ignore the filename component of the old path. */
  auto pos = old_dir.find_last_of(Fil_path::SEPARATOR);
  if (pos == std::string::npos) {
    old_dir = MySQL_datadir_path;
  } else {
    old_dir.resize(pos + 1);
    ut_ad(Fil_path::is_separator(old_dir.back()));
  }
  old_dir = Fil_path::get_real_path(old_dir);

  /* Build the new path from the scan path and the found path. */
  std::string new_dir{result.first};

  ut_ad(Fil_path::is_separator(new_dir.back()));

  new_dir.append(result.second->front());

  new_dir = Fil_path::get_real_path(new_dir);

  /* Do not use a datafile that is in the wrong place. */
  if (!Fil_path::is_valid_location(space_name, new_dir)) {
    ib::info(ER_IB_MSG_353)
        << "Cannot use scanned file " << new_dir << " for tablespace "
        << space_name << " because it is not in a valid location.";

    return (Fil_state::MISSING);
  }

  /* Ignore the filename component of the new path. */
  pos = new_dir.find_last_of(Fil_path::SEPARATOR);

  ut_ad(pos != std::string::npos);

  new_dir.resize(pos + 1);

  if (old_dir.compare(new_dir) != 0) {
    *new_path = result.first + result.second->front();

    fil_system->moved(dd_object_id, space_id, space_name, old_path, *new_path);

    return (Fil_state::MOVED);
  }

  *new_path = old_path;

  return (Fil_state::MATCHES);
}

#endif /* !UNIV_HOTBACKUP */

/** This function should be called after recovery has completed.
Check for tablespace files for which we did not see any MLOG_FILE_DELETE
or MLOG_FILE_RENAME record. These could not be recovered.
@return true if there were some filenames missing for which we had to
        ignore redo log records during the apply phase */
bool Fil_system::check_missing_tablespaces() {
  bool missing = false;
  auto &dblwr = recv_sys->dblwr;
  const auto end = recv_sys->deleted.end();

  /* Called in single threaded mode, no need to acquire the mutex. */

  /* First check if we were able to restore all the doublewrite
  buffer pages. If not then print a warning. */

  for (auto &page : dblwr.deferred) {
    space_id_t space_id;

    space_id = page_get_space_id(page.m_page);

    /* If the tablespace was in the missing IDs then we
    know that the problem is elsewhere. If a file deleted
    record was not found in the redo log and the tablespace
    doesn't exist in the SYS_TABLESPACES file then it is
    an error or data corruption. The special case is an
    undo truncate in progress. */

    if (recv_sys->deleted.find(space_id) == end &&
        recv_sys->missing_ids.find(space_id) != recv_sys->missing_ids.end()) {
      page_no_t page_no;

      page_no = page_get_page_no(page.m_page);

      ib::warn(ER_IB_MSG_1263)
          << "Doublewrite page " << page.m_no << " for {space: " << space_id
          << ", page_no:" << page_no << "} could not be restored."
          << " File name unknown for tablespace ID " << space_id;
    }

    /* Free the memory. */
    page.close();
  }

  dblwr.deferred.clear();

  for (auto space_id : recv_sys->missing_ids) {
    if (recv_sys->deleted.find(space_id) != end) {
      continue;
    }

    const auto result = get_scanned_files(space_id);

    if (result.second == nullptr) {
      if (fsp_is_undo_tablespace(space_id)) {
        /* This could happen if an undo truncate is in progress because
        undo tablespace construction is not redo logged.  The DD is updated
        at the end and may be out of sync. */
        continue;
      }

      ib::error(ER_IB_MSG_354) << "Could not find any file associated with"
                               << " the tablespace ID: " << space_id;
      missing = true;

    } else {
      ut_a(!result.second->empty());
    }
  }

  return (missing);
}



/** Redo a tablespace create.
@param[in]	ptr		redo log record
@param[in]	end		end of the redo log buffer
@param[in]	page_id		Tablespace Id and first page in file
@param[in]	parsed_bytes	Number of bytes parsed so far
@param[in]	parse_only	Don't apply, parse only
@return pointer to next redo log record
@retval nullptr if this log record was truncated */
byte *fil_tablespace_redo_create(byte *ptr, const byte *end,
                                 const page_id_t &page_id, ulint parsed_bytes,
                                 bool parse_only) {
  ut_a(page_id.page_no() == 0);

  /* We never recreate the system tablespace. */
  ut_a(page_id.space() != TRX_SYS_SPACE);

  ut_a(parsed_bytes != ULINT_UNDEFINED);

  /* Where 6 = flags (uint32_t) + name len (uint16_t). */
  if (end <= ptr + 6) {
    return (nullptr);
  }

#ifdef UNIV_HOTBACKUP
  uint32_t flags = mach_read_from_4(ptr);
#else
    /* Skip the flags, not used here. */
#endif /* UNIV_HOTBACKUP */

  ptr += 4;

  ulint len = mach_read_from_2(ptr);

  ptr += 2;

  /* Do we have the full/valid file name. */
  if (end < ptr + len || len < 5) {
    if (len < 5) {
      char name[6];

      snprintf(name, sizeof(name), "%.*s", (int)len, ptr);

      ib::error(ER_IB_MSG_355) << "MLOG_FILE_CREATE : Invalid file name."
                               << " Length (" << len << ") must be >= 5"
                               << " and end in '.ibd'. File name in the"
                               << " redo log is '" << name << "'";

      recv_sys->found_corrupt_log = true;
    }

    return (nullptr);
  }

  char *name = reinterpret_cast<char *>(ptr);

  Fil_path::normalize(name);

  ptr += len;

  if (!Fil_path::has_suffix(IBD, name)) {
    recv_sys->found_corrupt_log = true;

    return (nullptr);
  }

  if (parse_only) {
    return (ptr);
  }
#ifdef UNIV_HOTBACKUP

  meb_tablespace_redo_create(page_id, flags, name);

#else  /* !UNIV_HOTBACKUP */

  const auto result = fil_system->get_scanned_files(page_id.space());

  if (result.second == nullptr) {
    /* No file maps to this tablespace ID. It's possible that
    the file was deleted later or is misisng. */

    return (ptr);
  }

  auto abs_name = Fil_path::get_real_path(name);

  /* Duplicates should have been sorted out before we get here. */
  ut_a(result.second->size() == 1);

  /* It's possible that the tablespace file was renamed later. */
  if (result.second->front().compare(abs_name) == 0) {
    bool success;

    success = fil_tablespace_open_for_recovery(page_id.space());

    if (!success) {
      ib::info(ER_IB_MSG_356) << "Create '" << abs_name << "' failed!";
    }
  }
#endif /* UNIV_HOTBACKUP */

  return (ptr);
}

/** Redo a tablespace rename.
This function doesn't do anything, simply parses the redo log record.
@param[in]	ptr		redo log record
@param[in]	end		end of the redo log buffer
@param[in]	page_id		Tablespace Id and first page in file
@param[in]	parsed_bytes	Number of bytes parsed so far
@param[in]	parse_only	Don't apply, parse only
@return pointer to next redo log record
@retval nullptr if this log record was truncated */
byte *fil_tablespace_redo_rename(byte *ptr, const byte *end,
                                 const page_id_t &page_id, ulint parsed_bytes,
                                 bool parse_only) {
  ut_a(page_id.page_no() == 0);

  /* We never recreate the system tablespace. */
  ut_a(page_id.space() != TRX_SYS_SPACE);

  ut_a(parsed_bytes != ULINT_UNDEFINED);

  /* Where 2 = from name len (uint16_t). */
  if (end <= ptr + 2) {
    return (nullptr);
  }

  /* Read and check the RENAME FROM_NAME. */
  ulint from_len = mach_read_from_2(ptr);

  ptr += 2;

  /* Do we have the full/valid from and to file names. */
  if (end < ptr + from_len || from_len < 5) {
    if (from_len < 5) {
      char name[6];

      snprintf(name, sizeof(name), "%.*s", (int)from_len, ptr);

      ib::info(ER_IB_MSG_357) << "MLOG_FILE_RENAME: Invalid from file name."
                              << " Length (" << from_len << ") must be >= 5"
                              << " and end in '.ibd'. File name in the"
                              << " redo log is '" << name << "'";
    }

    return (nullptr);
  }

  char *from_name = reinterpret_cast<char *>(ptr);

  Fil_path::normalize(from_name);

  auto abs_from_name = Fil_path::get_real_path(from_name);

  ptr += from_len;

  if (!Fil_path::has_suffix(IBD, abs_from_name)) {
    ib::error(ER_IB_MSG_358)
        << "MLOG_FILE_RENAME: From file name doesn't end in"
        << " .ibd. File name in the redo log is '" << from_name << "'";

    recv_sys->found_corrupt_log = true;

    return (nullptr);
  }

  /* Read and check the RENAME TO_NAME. */

  ulint to_len = mach_read_from_2(ptr);

  ptr += 2;

  if (end < ptr + to_len || to_len < 5) {
    if (to_len < 5) {
      char name[6];

      snprintf(name, sizeof(name), "%.*s", (int)to_len, ptr);

      ib::info(ER_IB_MSG_359) << "MLOG_FILE_RENAME: Invalid to file name."
                              << " Length (" << to_len << ") must be >= 5"
                              << " and end in '.ibd'. File name in the"
                              << " redo log is '" << name << "'";
    }

    return (nullptr);
  }

  char *to_name = reinterpret_cast<char *>(ptr);

  ptr += to_len;

  Fil_path::normalize(to_name);

#ifdef UNIV_HOTBACKUP

  if (!parse_only) {
    meb_tablespace_redo_rename(page_id, from_name, to_name);
  }

#else  /* !UNIV_HOTBACKUP */

  auto abs_to_name = Fil_path::get_real_path(to_name);

  if (from_len == to_len && strncmp(to_name, from_name, to_len) == 0) {
    ib::error(ER_IB_MSG_360)
        << "MLOG_FILE_RENAME: The from and to name are the"
        << " same: '" << from_name << "', '" << to_name << "'";

    recv_sys->found_corrupt_log = true;

    return (nullptr);
  }

  if (!Fil_path::has_suffix(IBD, abs_to_name)) {
    ib::error(ER_IB_MSG_361)
        << "MLOG_FILE_RENAME: To file name doesn't end in"
        << " .ibd. File name in the redo log is '" << to_name << "'";

    recv_sys->found_corrupt_log = true;

    return (nullptr);
  }
#endif /* UNIV_HOTBACKUP */

  return (ptr);
}

/** Redo a tablespace delete.
@param[in]	ptr		redo log record
@param[in]	end		end of the redo log buffer
@param[in]	page_id		Tablespace Id and first page in file
@param[in]	parsed_bytes	Number of bytes parsed so far
@param[in]	parse_only	Don't apply, parse only
@return pointer to next redo log record
@retval nullptr if this log record was truncated */
byte *fil_tablespace_redo_delete(byte *ptr, const byte *end,
                                 const page_id_t &page_id, ulint parsed_bytes,
                                 bool parse_only) {
  ut_a(page_id.page_no() == 0);

  /* We never recreate the system tablespace. */
  ut_a(page_id.space() != TRX_SYS_SPACE);

  ut_a(parsed_bytes != ULINT_UNDEFINED);

  /* Where 2 =  len (uint16_t). */
  if (end <= ptr + 2) {
    return (nullptr);
  }

  ulint len = mach_read_from_2(ptr);

  ptr += 2;

  /* Do we have the full/valid file name. */
  if (end < ptr + len || len < 5) {
    if (len < 5) {
      char name[6];

      snprintf(name, sizeof(name), "%.*s", (int)len, ptr);

      ib::error(ER_IB_MSG_362) << "MLOG_FILE_DELETE : Invalid file name."
                               << " Length (" << len << ") must be >= 5"
                               << " and end in '.ibd'. File name in the"
                               << " redo log is '" << name << "'";
    }

    return (nullptr);
  }

  char *name = reinterpret_cast<char *>(ptr);

  Fil_path::normalize(name);

  ptr += len;

  if (!Fil_path::has_suffix(IBD, name)) {
    recv_sys->found_corrupt_log = true;

    return (nullptr);
  }

  if (parse_only) {
    return (ptr);
  }
#ifdef UNIV_HOTBACKUP

  meb_tablespace_redo_delete(page_id, name);

#else  /* !UNIV_HOTBACKUP */

  const auto result = fil_system->get_scanned_files(page_id.space());

  recv_sys->deleted.insert(page_id.space());
  recv_sys->missing_ids.erase(page_id.space());

  if (result.second == nullptr) {
    /* No files map to this tablespace ID. The drop must
    have succeeded. */

    return (ptr);
  }

  /* Space_id_set should have been sorted out before we get here. */

  ut_a(result.second->size() == 1);

  auto abs_name = Fil_path::get_real_path(name);

  ut_ad(!Fil_path::is_separator(abs_name.back()));

  fil_space_free(page_id.space(), false);

  bool success = fil_system->erase(page_id.space());
  ut_a(success);
#endif /* UNIV_HOTBACKUP */

  return (ptr);
}

/** Parse and process an encryption redo record.
@param[in]	ptr		redo log record
@param[in]	end		end of the redo log buffer
@param[in]	space_id	the tablespace ID
@return log record end, nullptr if not a complete record */
byte *fil_tablespace_redo_encryption(byte *ptr, const byte *end,
                                     space_id_t space_id) {
  byte *iv = nullptr;
  byte *key = nullptr;
  bool is_new = false;

#ifdef UNIV_DEBUG
  bool is_allocated = false;
#endif

  fil_space_t *space = fil_space_get(space_id);

  /* An undo space might be open but not have the ENCRYPTION bit set
  in its header if the current value of innodb_undo_log_encrypt=OFF
  and a crash occured between flushing this redo record and the header
  page of the undo space.  So if the flag is missing, ignore the header
  page. */
  if (fsp_is_undo_tablespace(space_id) && space != nullptr &&
      !FSP_FLAGS_GET_ENCRYPTION(space->flags)) {
    space = nullptr;
  }

  if (space == nullptr) {
    if (recv_sys->keys == nullptr) {
      recv_sys->keys = UT_NEW_NOKEY(recv_sys_t::Encryption_Keys());
    }

    for (auto &recv_key : *recv_sys->keys) {
      if (recv_key.space_id == space_id) {
        iv = recv_key.iv;
        key = recv_key.ptr;
      }
    }

#ifdef UNIV_DEBUG
    if (key != nullptr) {
      DBUG_EXECUTE_IF(
          "dont_update_key_found_during_REDO_scan", is_allocated = true;
          key = static_cast<byte *>(ut_malloc_nokey(ENCRYPTION_KEY_LEN));
          iv = static_cast<byte *>(ut_malloc_nokey(ENCRYPTION_KEY_LEN)););
    }
#endif

    if (key == nullptr) {
      key = static_cast<byte *>(ut_malloc_nokey(ENCRYPTION_KEY_LEN));

      iv = static_cast<byte *>(ut_malloc_nokey(ENCRYPTION_KEY_LEN));

      is_new = true;
    }

  } else {
    iv = space->encryption_iv;
    key = space->encryption_key;
  }

  ulint offset;

  offset = mach_read_from_2(ptr);
  ptr += 2;

  ulint len;

  len = mach_read_from_2(ptr);
  ptr += 2;

  if (end < ptr + len) {
    return (nullptr);
  }

  if (offset >= UNIV_PAGE_SIZE || len + offset > UNIV_PAGE_SIZE ||
      len != ENCRYPTION_INFO_SIZE) {
    recv_sys->found_corrupt_log = true;
    return (nullptr);
  }

  if (!Encryption::decode_encryption_info(key, iv, ptr)) {
    recv_sys->found_corrupt_log = true;

    ib::warn(ER_IB_MSG_364)
        << "Encryption information"
        << " in the redo log of space " << space_id << " is invalid";

    return (nullptr);
  }

  ut_ad(len == ENCRYPTION_INFO_SIZE);

  ptr += len;

  if (space == nullptr) {
    if (is_new) {
      recv_sys_t::Encryption_Key new_key;

      new_key.iv = iv;
      new_key.ptr = key;
      new_key.space_id = space_id;

      recv_sys->keys->push_back(new_key);
    }
  } else {
    if (FSP_FLAGS_GET_ENCRYPTION(space->flags) ||
        space->encryption_op_in_progress == ENCRYPTION) {
      space->encryption_type = Encryption::AES;
      space->encryption_klen = ENCRYPTION_KEY_LEN;
    }
  }

#ifdef UNIV_DEBUG
  if (is_allocated) {
    DBUG_EXECUTE_IF("dont_update_key_found_during_REDO_scan", ut_free(key);
                    ut_free(iv););
  }
#endif

  return (ptr);
}

/** Check whether we can rename the file
@param[in]	space		Tablespace for which to rename
@param[in]	name		Source file name
@param[in]	df		Target file that exists on disk
@return DB_SUCCESS if all OK */
static dberr_t fil_rename_validate(fil_space_t *space, const std::string &name,
                                   Datafile &df) {
  dberr_t err = df.validate_for_recovery(space->id);

  if (err == DB_TABLESPACE_NOT_FOUND) {
    /* Tablespace header doesn't contain the expected
    tablespace ID. This is can happen during truncate. */

    return (err);

  } else if (err != DB_SUCCESS) {
    ib::warn(ER_IB_MSG_367) << "Failed to read the first page of the"
                            << " file '" << df.filepath() << "'."
                            << " You will need to verify and move the"
                            << " file out of the way retry recovery.";

    return (err);
  }

  auto file = &space->files.front();

  if (strcmp(df.filepath(), file->name) == 0) {
    /* Check if already points to the correct file.
    Must have the same space ID */

    ib::info(ER_IB_MSG_368) << "Tablespace ID already maps to: '"
                            << df.filepath() << "', rename ignored.";

    ut_a(df.space_id() == space->id);

    return (DB_SUCCESS);

  } else if (df.space_id() != space->id) {
    /* Target file exists on disk but has a different
    tablespce ID. The user should manually delete it. */

    ib::error(ER_IB_MSG_369)
        << "Cannot rename '" << name << "' to '" << df.filepath() << "'. File '"
        << df.filepath() << "' tablespace ID " << df.space_id()
        << " doesn't match the expected tablespace"
        << " ID " << space->id << ". You will need to verify and move '"
        << df.filepath() << "' manually and retry recovery!";

    return (DB_ERROR);
  }

  /* Target file exists on disk and has the same ID. */

  ib::error(ER_IB_MSG_370)
      << "Cannot rename '" << name << "' to '" << df.filepath()
      << "'. The File '" << df.filepath() << " already exists on"
      << " disk. You will need to verify and move either file"
      << " manually and retry recovery!";

  return (DB_ERROR);
}

/** Replay a file rename operation if possible.
@param[in]	page_id		Space ID and first page number in the file
@param[in]	old_name	old file name
@param[in]	new_name	new file name
@return	whether the operation was successfully applied (the name did not exist,
or new_name did not exist and name was successfully renamed to new_name)  */
static bool fil_op_replay_rename(const page_id_t &page_id,
                                 const std::string &old_name,
                                 const std::string &new_name) {
#ifdef UNIV_HOTBACKUP
  ut_ad(meb_replay_file_ops);
#endif /* UNIV_HOTBACKUP */

  ut_ad(page_id.page_no() == 0);
  ut_ad(old_name.compare(new_name) != 0);
  ut_ad(Fil_path::has_suffix(IBD, new_name));
  ut_ad(page_id.space() != TRX_SYS_SPACE);

  /* In order to replay the rename, the following must hold:
  1. The new name is not already used.
  2. A tablespace exists with the old name.
  3. The space ID for that tablepace matches this log entry.
  This will prevent unintended renames during recovery. */

  space_id_t space_id = page_id.space();
  fil_space_t *space = fil_space_get(space_id);

  if (space == nullptr) {
    return (true);
  }

  Datafile df;
  std::string name{new_name};

  df.set_filepath(name.c_str());

  if (df.open_read_only(false) == DB_SUCCESS) {
    dberr_t err = fil_rename_validate(space, old_name, df);

    if (err == DB_TABLESPACE_NOT_FOUND) {
      /* This can happend during truncate. */
      ib::info(ER_IB_MSG_371) << "Tablespace ID mismatch in '" << name << "'";
    }

    df.close();

    return (err == DB_SUCCESS);
  }

  auto path_sep_pos = name.find_last_of(Fil_path::SEPARATOR);

  ut_a(path_sep_pos != std::string::npos);

  /* Create the database directory for the new name, if
  it does not exist yet */

  name.resize(path_sep_pos);

  bool success = os_file_create_directory(name.c_str(), false);
  ut_a(success);

  auto datadir_pos = name.find_last_of(Fil_path::SEPARATOR);

  ut_ad(datadir_pos != std::string::npos);

  name.erase(0, datadir_pos + 1);

  ut_ad(!Fil_path::is_separator(name.back()));

  /* schema/table separator is always a '/'. */
  name.push_back('/');

  /* Strip the '.ibd' suffix. */
  name.append(new_name.begin() + path_sep_pos + 1, new_name.end() - 4);

  ut_ad(!Fil_path::has_suffix(IBD, name));

  clone_mark_abort(true);

  const auto ptr = name.c_str();

  dberr_t err =
      fil_rename_tablespace(space_id, old_name.c_str(), ptr, new_name.c_str());

  /* Stop recovery if this does not succeed. */
  ut_a(err == DB_SUCCESS);

  clone_mark_active();

  return (true);
}



/** Discover tablespaces by reading the header from .ibd files.
@param[in]	in_directories	Directories to scan
@return DB_SUCCESS if all goes well */
dberr_t Tablespace_dirs::scan(const std::string &in_directories) {
  std::string directories(in_directories);

  Fil_path::normalize(directories);

  ib::info(ER_IB_MSG_378) << "Directories to scan '" << directories << "'";

  Scanned_files ibd_files;
  Scanned_files undo_files;

  {
    std::string separators;

    separators.push_back(FIL_PATH_SEPARATOR);

    tokenize_paths(directories, separators);
  }

  uint16_t count = 0;
  bool print_msg = false;
  auto start_time = ut_time();

  /* Should be trivial to parallelize the scan and ID check. */
  for (const auto &dir : m_dirs) {
    const auto &real_path_dir = dir.real_path();

    ut_a(Fil_path::is_separator(dir.path().back()));

    ib::info(ER_IB_MSG_379) << "Scanning '" << dir.path() << "'";

    /* Walk the sub-tree of dir. */

    Dir_Walker::walk(real_path_dir, true, [&](const std::string &path) {
      /* If it is a file and the suffix matches ".ibd"
      or the undo file name format then store it for
      determining the space ID. */

      ut_a(path.length() > real_path_dir.length());
      ut_a(Fil_path::get_file_type(path) != OS_FILE_TYPE_DIR);

      /* Make the filename relative to the directory that
      was scanned. */

      std::string file = path.substr(real_path_dir.length(), path.length());

      if (file.size() <= 4) {
        return;
      }

      using value = Scanned_files::value_type;

      if (Fil_path::has_suffix(IBD, file.c_str())) {
        ibd_files.push_back(value{count, file});

      } else if (Fil_path::is_undo_tablespace_name(file)) {
        undo_files.push_back(value{count, file});
      }

      if (ut_time() - start_time >= PRINT_INTERVAL_SECS) {
        ib::info(ER_IB_MSG_380)
            << "Files found so far: " << ibd_files.size() << " data files"
            << " and " << undo_files.size() << " undo files";

        start_time = ut_time();
        print_msg = true;
      }
    });

    ++count;
  }

  if (print_msg) {
    ib::info(ER_IB_MSG_381) << "Found " << ibd_files.size() << " '.ibd' and "
                            << undo_files.size() << " undo files";
  }

  Space_id_set unique;
  Space_id_set duplicates;

  size_t n_threads = (ibd_files.size() / 50000);

  if (n_threads > 0) {
    if (n_threads > MAX_SCAN_THREADS) {
      n_threads = MAX_SCAN_THREADS;
    }

    ib::info(ER_IB_MSG_382) << "Using " << (n_threads + 1) << " threads to"
                            << " scan the tablespace files";
  }

  std::mutex m;

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  using std::placeholders::_4;
  using std::placeholders::_5;
  using std::placeholders::_6;

  std::function<void(const Const_iter &, const Const_iter &, size_t,
                     std::mutex *, Space_id_set *, Space_id_set *)>
      check = std::bind(&Tablespace_dirs::duplicate_check, this, _1, _2, _3, _4,
                        _5, _6);

  par_for(PFS_NOT_INSTRUMENTED, ibd_files, n_threads, check, &m, &unique,
          &duplicates);

  duplicate_check(undo_files.begin(), undo_files.end(), n_threads, &m, &unique,
                  &duplicates);

  ut_a(m_checked == ibd_files.size() + undo_files.size());

  ib::info(ER_IB_MSG_383) << "Completed space ID check of " << m_checked.load()
                          << " files.";

  dberr_t err;

  if (!duplicates.empty()) {
    ib::error(ER_IB_MSG_384)
        << "Multiple files found for the same tablespace ID:";

    print_duplicates(duplicates);

    err = DB_FAIL;
  } else {
    err = DB_SUCCESS;
  }

  return (err);
}



/** Callback to check tablespace size with space header size and extend.
Caller must own the Fil_shard mutex that the file belongs to.
@param[in]	file	Tablespace file
@return	error code */
dberr_t fil_check_extend_space(fil_node_t *file) {
  dberr_t err = DB_SUCCESS;
  bool open_node = !file->is_open;

  if (recv_sys == nullptr || !recv_sys->is_cloned_db) {
    return (DB_SUCCESS);
  }

  fil_space_t *space = file->space;

  auto shard = fil_system->shard_by_id(space->id);

  if (open_node && !shard->open_file(file, false)) {
    return (DB_CANNOT_OPEN_FILE);
  }

  shard->mutex_release();

  if (space->size < space->size_in_header) {
    ib::info(ER_IB_MSG_385)
        << "Extending space: " << space->name << " from size " << space->size
        << " pages to " << space->size_in_header
        << " pages as stored in space header.";

    if (!shard->space_extend(space, space->size_in_header)) {
      ib::error(ER_IB_MSG_386) << "Failed to extend tablespace."
                               << " Check for free space in disk"
                               << " and try again.";

      err = DB_OUT_OF_FILE_SPACE;
    }
  }

  shard->mutex_acquire();

  /* Close the file if it was opened by current function */
  if (open_node) {
    shard->close_file(file, true);
  }

  return (err);
}









