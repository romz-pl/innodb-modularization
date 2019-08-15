#include <innodb/tablespace/Fil_shard.h>

#include <innodb/dict_mem/flags.h>
#include <innodb/tablespace/fsp_is_session_temporary.h>
#include <innodb/tablespace/fsp_is_global_temporary.h>
#include <innodb/tablespace/fsp_is_system_tablespace.h>
#include <innodb/sync_os/pfs.h>
#include <innodb/sync_event/os_event_wait_low.h>
#include <innodb/io/os_file_flush.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/tablespace/fil_n_log_flushes.h>
#include <innodb/tablespace/fil_n_pending_log_flushes.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/error/ut_error.h>
#include <innodb/formatting/hex.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/IORequestRead.h>
#include <innodb/io/access_type.h>
#include <innodb/io/pfs.h>
#include <innodb/io/pfs.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_create.h>
#include <innodb/io/os_file_create_simple_no_error_handling.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/os_file_get_last_error.h>
#include <innodb/io/os_file_get_size.h>
#include <innodb/io/os_file_get_status.h>
#include <innodb/io/os_file_stat_t.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/logger/fatal.h>
#include <innodb/logger/info.h>
#include <innodb/math/ut_2pow_round.h>
#include <innodb/string/mem_strdup.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_rw/rw_lock_create.h>
#include <innodb/sync_rw/rw_lock_free.h>
#include <innodb/tablespace/Fil_system.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fil_buffering_disabled.h>
#include <innodb/tablespace/fil_n_file_opened.h>
#include <innodb/tablespace/fil_no_punch_hole.h>
#include <innodb/tablespace/flst_get_len.h>
#include <innodb/tablespace/fsp_header_get_field.h>
#include <innodb/tablespace/fsp_header_get_flags.h>
#include <innodb/tablespace/fsp_header_get_space_id.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/os_file_read_first_page.h>
#include <innodb/tablespace/srv_start_raw_disk_in_use.h>
#include <innodb/time/ib_time_t.h>
#include <innodb/time/ut_time.h>
#include <innodb/log_types/recv_recovery_on.h>
#include <innodb/tablespace/fil_write_zeros.h>
#include <innodb/io/os_has_said_disk_full.h>
#include <innodb/io/IORequestWrite.h>
#include <innodb/tablespace/srv_sys_space.h>
#include <innodb/tablespace/fsp_is_system_temporary.h>
#include <innodb/tablespace/srv_tmp_space.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/tablespace/fsp_is_system_temporary.h>
#include <innodb/ioasync/os_aio_simulated_wake_handler_threads.h>
#include <innodb/tablespace/fil_system.h>
#include <innodb/enum/to_int.h>
#include <innodb/tablespace/dict_sys_t_is_reserved.h>

#include <atomic>


namespace undo {
bool is_active(space_id_t space_id, bool get_latch);
}

bool recv_recovery_is_on();



extern bool srv_startup_is_before_trx_rollback_phase;
extern ulong srv_fast_shutdown;

/** Total number of open files. */
std::atomic_size_t Fil_shard::s_n_open;

/** Slot reserved for opening a file. */
std::atomic_size_t Fil_shard::s_open_slot;


#ifdef UNIV_DEBUG
/** Validate a shard */
void Fil_shard::validate() const {
  mutex_acquire();

  size_t n_open = 0;

  for (auto elem : m_spaces) {
    page_no_t size = 0;
    auto space = elem.second;

    for (const auto &file : space->files) {
      ut_a(file.is_open || !file.n_pending);

      if (file.is_open) {
        ++n_open;
      }

      size += file.size;
    }

    ut_a(space->size == size);
  }

  UT_LIST_CHECK(m_LRU);

  for (auto file = UT_LIST_GET_FIRST(m_LRU); file != nullptr;
       file = UT_LIST_GET_NEXT(LRU, file)) {
    ut_a(file->is_open);
    ut_a(file->n_pending == 0);
    ut_a(fil_system->space_belongs_in_LRU(file->space));
  }

  mutex_release();
}

#endif /* UNIV_DEBUG */

/** Constructor
@param[in]	shard_id	Shard ID  */
Fil_shard::Fil_shard(size_t shard_id)
    : m_id(shard_id), m_spaces(), m_names(), m_modification_counter() {
  mutex_create(LATCH_ID_FIL_SHARD, &m_mutex);

  UT_LIST_INIT(m_LRU, &fil_node_t::LRU);

  UT_LIST_INIT(m_unflushed_spaces, &fil_space_t::unflushed_spaces);
}

/** Wait for an empty slot to reserve for opening a file.
@return true on success. */
bool Fil_shard::reserve_open_slot(size_t shard_id) {
  size_t expected = EMPTY_OPEN_SLOT;

  return (s_open_slot.compare_exchange_weak(expected, shard_id));
}

/** Release the slot reserved for opening a file.
@param[in]	shard_id	ID of shard relasing the slot */
void Fil_shard::release_open_slot(size_t shard_id) {
  size_t expected = shard_id;

  while (!s_open_slot.compare_exchange_weak(expected, EMPTY_OPEN_SLOT)) {
  };
}

/** Map the space ID and name to the tablespace instance.
@param[in]	space		Tablespace instance */
void Fil_shard::space_add(fil_space_t *space) {
  ut_ad(mutex_owned());

  {
    auto it = m_spaces.insert(Spaces::value_type(space->id, space));

    ut_a(it.second);
  }

  {
    auto name = space->name;

    auto it = m_names.insert(Names::value_type(name, space));

    ut_a(it.second);
  }
}

/** Add the file node to the LRU list if required.
@param[in,out]	file		File for the tablespace */
void Fil_shard::file_opened(fil_node_t *file) {
  ut_ad(m_id == REDO_SHARD || mutex_owned());

  if (Fil_system::space_belongs_in_LRU(file->space)) {
    /* Put the file to the LRU list */
    UT_LIST_ADD_FIRST(m_LRU, file);
  }

  ++s_n_open;

  file->is_open = true;

  fil_n_file_opened = s_n_open;
}

/** Remove the file node from the LRU list.
@param[in,out]	file		File for the tablespace */
void Fil_shard::remove_from_LRU(fil_node_t *file) {
  ut_ad(mutex_owned());

  if (Fil_system::space_belongs_in_LRU(file->space)) {
    ut_ad(mutex_owned());

    ut_a(UT_LIST_GET_LEN(m_LRU) > 0);

    /* The file is in the LRU list, remove it */
    UT_LIST_REMOVE(m_LRU, file);
  }
}

/** Close a tablespace file based on tablespace ID.
@param[in]	space_id	Tablespace ID
@return false if space_id was not found. */
bool Fil_shard::close_file(space_id_t space_id) {
  mutex_acquire();

  auto space = get_space_by_id(space_id);

  if (space == nullptr) {
    mutex_release();

    return (false);
  }

  for (auto &file : space->files) {
    while (file.in_use > 0) {
      mutex_release();

      os_thread_sleep(10000);

      mutex_acquire();
    }

    if (file.is_open) {
      close_file(&file, false);
    }
  }

  mutex_release();

  return (true);
}

/** Remap the tablespace to the new name.
@param[in]	space		Tablespace instance, with old name.
@param[in]	new_name	New tablespace name */
void Fil_shard::update_space_name_map(fil_space_t *space,
                                      const char *new_name) {
  ut_ad(mutex_owned());

  ut_ad(m_spaces.find(space->id) != m_spaces.end());

  m_names.erase(space->name);

  auto it = m_names.insert(Names::value_type(new_name, space));

  ut_a(it.second);
}

/** Checks if all the file nodes in a space are flushed. The caller must hold
the fil_system mutex.
@param[in]	space		Tablespace to check
@return true if all are flushed */
bool Fil_shard::space_is_flushed(const fil_space_t *space) {
  ut_ad(mutex_owned());

  for (const auto &file : space->files) {
    if (file.modification_counter > file.flush_counter) {
      ut_ad(!fil_buffering_disabled(space));
      return (false);
    }
  }

  return (true);
}

/** Attach a file to a tablespace
@param[in]	name		file name of a file that is not open
@param[in]	size		file size in entire database blocks
@param[in,out]	space		tablespace from fil_space_create()
@param[in]	is_raw		whether this is a raw device or partition
@param[in]	punch_hole	true if supported for this file
@param[in]	atomic_write	true if the file has atomic write enabled
@param[in]	max_pages	maximum number of pages in file
@return pointer to the file name
@retval nullptr if error */
fil_node_t *Fil_shard::create_node(const char *name, page_no_t size,
                                   fil_space_t *space, bool is_raw,
                                   bool punch_hole, bool atomic_write,
                                   page_no_t max_pages) {
  ut_ad(name != nullptr);
  ut_ad(fil_system != nullptr);

  if (space == nullptr) {
    return (nullptr);
  }

  fil_node_t file;

  memset(&file, 0x0, sizeof(file));

  file.name = mem_strdup(name);

  ut_a(!is_raw || srv_start_raw_disk_in_use);

  file.sync_event = os_event_create("fsync_event");

  file.is_raw_disk = is_raw;

  file.size = size;

  file.flush_size = size;

  file.magic_n = FIL_NODE_MAGIC_N;

  file.init_size = size;

  file.max_size = max_pages;

  file.space = space;

  os_file_stat_t stat_info;

#ifdef UNIV_DEBUG
  dberr_t err =
#endif /* UNIV_DEBUG */

      os_file_get_status(
          file.name, &stat_info, false,
          fsp_is_system_temporary(space->id) ? true : srv_read_only_mode);

  ut_ad(err == DB_SUCCESS);

  file.block_size = stat_info.block_size;

  /* In this debugging mode, we can overcome the limitation of some
  OSes like Windows that support Punch Hole but have a hole size
  effectively too large.  By setting the block size to be half the
  page size, we can bypass one of the checks that would normally
  turn Page Compression off.  This execution mode allows compression
  to be tested even when full punch hole support is not available. */
  DBUG_EXECUTE_IF(
      "ignore_punch_hole",
      file.block_size = std::min(static_cast<ulint>(stat_info.block_size),
                               UNIV_PAGE_SIZE / 2););

  if (!IORequest::is_punch_hole_supported() || !punch_hole ||
      file.block_size >= srv_page_size) {
    fil_no_punch_hole(&file);
  } else {
    file.punch_hole = punch_hole;
  }

  file.atomic_write = atomic_write;

  mutex_acquire();

  space->size += size;

  space->files.push_back(file);

  mutex_release();

  ut_a(space->id == TRX_SYS_SPACE ||
       space->id == dict_sys_t_s_log_space_first_id ||
       space->purpose == FIL_TYPE_TEMPORARY || space->files.size() == 1);

  return (&space->files.front());
}



/** First we open the file in the normal mode, no async I/O here, for
simplicity. Then do some checks, and close the file again.  NOTE that we
could not use the simple file read function os_file_read() in Windows
to read from a file opened for async I/O!
@param[in,out]	file		Get the size of this file
@param[in]	read_only_mode	true if read only mode set
@return DB_SUCCESS or error */
dberr_t Fil_shard::get_file_size(fil_node_t *file, bool read_only_mode) {
  bool success;
  fil_space_t *space = file->space;

  do {
    ut_a(!file->is_open);

    file->handle = os_file_create_simple_no_error_handling(
        innodb_data_file_key, file->name, OS_FILE_OPEN, OS_FILE_READ_ONLY,
        read_only_mode, &success);

    if (!success) {
      /* The following call prints an error message */
      ulint err = os_file_get_last_error(true);

      if (err == EMFILE + 100) {
        if (close_files_in_LRU(true)) {
          continue;
        }
      }

      ib::warn(ER_IB_MSG_268) << "Cannot open '" << file->name
                              << "'."
                                 " Have you deleted .ibd files under a"
                                 " running mysqld server?";

      return (DB_ERROR);
    }

  } while (!success);

  os_offset_t size_bytes = os_file_get_size(file->handle);

  ut_a(size_bytes != (os_offset_t)-1);

#ifdef UNIV_HOTBACKUP
  if (space->id == TRX_SYS_SPACE) {
    file->size = (ulint)(size_bytes / UNIV_PAGE_SIZE);
    space->size += file->size;
    os_file_close(file->handle);
    return (DB_SUCCESS);
  }
#endif /* UNIV_HOTBACKUP */

  ut_a(space->purpose != FIL_TYPE_LOG);

  /* Read the first page of the tablespace */

  byte *buf2 = static_cast<byte *>(ut_malloc_nokey(2 * UNIV_PAGE_SIZE));

  /* Align memory for file I/O if we might have O_DIRECT set */

  byte *page = static_cast<byte *>(ut_align(buf2, UNIV_PAGE_SIZE));

  ut_ad(page == page_align(page));

  IORequest request(IORequest::READ);

  dberr_t err =
      os_file_read_first_page(request, file->handle, page, UNIV_PAGE_SIZE);

  ut_a(err == DB_SUCCESS);

  os_file_close(file->handle);

  uint32_t flags = fsp_header_get_flags(page);
  space_id_t space_id = fsp_header_get_space_id(page);

  /* To determine if tablespace is from 5.7 or not, we
  rely on SDI flag. For IBDs from 5.7, which are opened
  during import or during upgrade, their initial size
  is lesser than the initial size in 8.0 */
  bool has_sdi = FSP_FLAGS_HAS_SDI(flags);

  uint8_t expected_size =
      has_sdi ? FIL_IBD_FILE_INITIAL_SIZE : FIL_IBD_FILE_INITIAL_SIZE_5_7;

  const page_size_t page_size(flags);

  ulint min_size = expected_size * page_size.physical();

  if (size_bytes < min_size) {
    if (has_sdi) {
      /** Add some tolerance when the tablespace is upgraded. If an empty
      general tablespace is created in 5.7, and then upgraded to 8.0, then
      its size changes from FIL_IBD_FILE_INITIAL_SIZE_5_7 pages to
      FIL_IBD_FILE_INITIAL_SIZE-1. */

      ut_ad(expected_size == FIL_IBD_FILE_INITIAL_SIZE);
      ulint upgrade_size = (expected_size - 1) * page_size.physical();

      if (size_bytes < upgrade_size) {
        ib::error(ER_IB_MSG_269)
            << "The size of tablespace file " << file->name << " is only "
            << size_bytes << ", should be at least " << upgrade_size << "!";

        ut_error;
      }

    } else {
      ib::error(ER_IB_MSG_269)
          << "The size of tablespace file " << file->name << " is only "
          << size_bytes << ", should be at least " << min_size << "!";

      ut_error;
    }
  }

  if (space_id != space->id) {
    ib::fatal(ER_IB_MSG_270) << "Tablespace id is " << space->id
                             << " in the data dictionary but in file "
                             << file->name << " it is " << space_id << "!";
  }

  /* We need to adjust for compressed pages. */
  const page_size_t space_page_size(space->flags);

  if (!page_size.equals_to(space_page_size)) {
    ib::fatal(ER_IB_MSG_271)
        << "Tablespace file " << file->name << " has page size " << page_size
        << " (flags=" << ib::hex(flags) << ") but the data dictionary expects"
        << " page size " << space_page_size
        << " (flags=" << ib::hex(space->flags) << ")!";
  }

  /* If the SDI flag is set in the file header page, set it in space->flags. */
  space->flags |= flags & FSP_FLAGS_MASK_SDI;

#ifndef UNIV_HOTBACKUP
  /* It is possible that
  - For general tablespace, encryption flag is updated on disk but server
  crashed before DD could be updated OR
  - For DD tablespace, encryption flag is updated on disk.
  */
  if (FSP_FLAGS_GET_ENCRYPTION(flags)) {
    space->flags |= flags & FSP_FLAGS_MASK_ENCRYPTION;
  }
#endif /* UNIV_HOTBACKUP */

  /* Make a copy of space->flags and flags from the page header
  so that they can be compared. */
  uint32_t fil_space_flags = space->flags;
  uint32_t header_fsp_flags = flags;

  /* If a crash occurs while an UNDO space is being truncated,
  it will be created new at startup. In that case, the fil_space_t
  object will have the ENCRYPTION flag set, but the header page will
  not be marked until the srv_master_thread gets around to it.
  The opposite can occur where the header page contains the encryption
  flag but the fil_space_t does not.  It could happen that undo
  encryption was turned off just before the crash or shutdown so that
  the srv_master_thread did not yet have time to apply it.
  So don't compare the encryption flag for undo tablespaces. */
  if (fsp_is_undo_tablespace(space->id)) {
    fsp_flags_unset_encryption(fil_space_flags);
    fsp_flags_unset_encryption(header_fsp_flags);
  }

  /* Make sure the space_flags are the same as the header page flags. */
  if (fil_space_flags != header_fsp_flags) {
    ib::error(ER_IB_MSG_272, ulong{space->flags}, file->name, ulonglong{flags});
    ut_error;
  }

  {
    page_no_t size = fsp_header_get_field(page, FSP_SIZE);

    page_no_t free_limit;

    free_limit = fsp_header_get_field(page, FSP_FREE_LIMIT);

    ulint free_len;

    free_len = flst_get_len(FSP_HEADER_OFFSET + FSP_FREE + page);

    ut_ad(space->free_limit == 0 || space->free_limit == free_limit);

    ut_ad(space->free_len == 0 || space->free_len == free_len);

    space->size_in_header = size;
    space->free_limit = free_limit;

    ut_a(free_len < std::numeric_limits<uint32_t>::max());

    space->free_len = (uint32_t)free_len;
  }

  ut_free(buf2);

  /* For encrypted tablespace, we need to check the
  encryption key and iv(initial vector) is read. */
  if (FSP_FLAGS_GET_ENCRYPTION(space->flags) && !recv_recovery_is_on() &&
      space->encryption_type != Encryption::AES) {
    ib::error(ER_IB_MSG_273, file->name);

    return (DB_ERROR);
  }

  if (file->size == 0) {
    ulint extent_size;

    extent_size = page_size.physical() * FSP_EXTENT_SIZE;

#ifndef UNIV_HOTBACKUP
    /* Truncate the size to a multiple of extent size. */
    if (size_bytes >= extent_size) {
      size_bytes = ut_2pow_round(size_bytes, extent_size);
    }
#else /* !UNIV_HOTBACKUP */

    /* After apply-incremental, tablespaces are not
    extended to a whole megabyte. Do not cut off
    valid data. */

#endif /* !UNIV_HOTBACKUP */

    file->size = static_cast<page_no_t>(size_bytes / page_size.physical());

    space->size += file->size;
  }

  return (DB_SUCCESS);
}

/** Open a file of a tablespace.
The caller must own the shard mutex.
@param[in,out]	file		Tablespace file
@param[in]	extend		true if the file is being extended
@return false if the file can't be opened, otherwise true */
bool Fil_shard::open_file(fil_node_t *file, bool extend) {
  bool success;
  fil_space_t *space = file->space;

  ut_ad(m_id == REDO_SHARD || mutex_owned());

  ut_a(!file->is_open);
  ut_a(file->n_pending == 0);

  while (file->in_use > 0) {
    /* We increment the reference count when extending
    the file. */
    if (file->in_use == 1 && extend) {
      break;
    }

    mutex_release();

    os_thread_sleep(100000);

    mutex_acquire();
  }

  if (file->is_open) {
    return (true);
  }

  bool read_only_mode;

  read_only_mode = !fsp_is_system_temporary(space->id) && srv_read_only_mode;

  if (file->size == 0 ||
      (space->size_in_header == 0 && space->purpose == FIL_TYPE_TABLESPACE &&
       file == &space->files.front()
#ifndef UNIV_HOTBACKUP
       && undo::is_active(space->id, false) &&
       srv_startup_is_before_trx_rollback_phase
#endif /* !UNIV_HOTBACKUP */
       )) {

    /* We don't know the file size yet. */
    dberr_t err = get_file_size(file, read_only_mode);

    if (err != DB_SUCCESS) {
      return (false);
    }
  }

  /* Open the file for reading and writing, in Windows normally in the
  unbuffered async I/O mode, though global variables may make
  os_file_create() to fall back to the normal file I/O mode. */

  if (space->purpose == FIL_TYPE_LOG) {
    file->handle =
        os_file_create(innodb_log_file_key, file->name, OS_FILE_OPEN,
                       OS_FILE_AIO, OS_LOG_FILE, read_only_mode, &success);
  } else if (file->is_raw_disk) {
    file->handle =
        os_file_create(innodb_data_file_key, file->name, OS_FILE_OPEN_RAW,
                       OS_FILE_AIO, OS_DATA_FILE, read_only_mode, &success);
  } else {
    file->handle =
        os_file_create(innodb_data_file_key, file->name, OS_FILE_OPEN,
                       OS_FILE_AIO, OS_DATA_FILE, read_only_mode, &success);
  }

  ut_a(success);

  /* The file is ready for IO. */

  file_opened(file);

  return (true);
}


/** Close a tablespace file.
@param[in]	LRU_close	true if called from LRU close
@param[in,out]	file		Tablespace file to close */
void Fil_shard::close_file(fil_node_t *file, bool LRU_close) {
  ut_ad(mutex_owned());

  ut_a(file->is_open);
  ut_a(file->in_use == 0);
  ut_a(file->n_pending == 0);
  ut_a(file->n_pending_flushes == 0);

#ifndef UNIV_HOTBACKUP
  ut_a(file->modification_counter == file->flush_counter ||
       file->space->purpose == FIL_TYPE_TEMPORARY || srv_fast_shutdown == 2);
#endif /* !UNIV_HOTBACKUP */

  bool ret = os_file_close(file->handle);

  ut_a(ret);

  file->handle.m_file = (os_file_t)-1;

  file->is_open = false;

  ut_a(s_n_open > 0);

  --s_n_open;

  fil_n_file_opened = s_n_open;

  remove_from_LRU(file);
}

/** Tries to close a file in the LRU list.
@param[in]	print_info	if true, prints information why it cannot close
                                a file
@return true if success, false if should retry later */
bool Fil_shard::close_files_in_LRU(bool print_info) {
  ut_ad(mutex_owned());

  for (auto file = UT_LIST_GET_LAST(m_LRU); file != nullptr;
       file = UT_LIST_GET_PREV(LRU, file)) {
    if (file->modification_counter == file->flush_counter &&
        file->n_pending_flushes == 0 && file->in_use == 0) {
      close_file(file, true);

      return (true);
    }

    if (!print_info) {
      continue;
    }

    if (file->n_pending_flushes > 0) {
      ib::info(ER_IB_MSG_274, file->name, file->n_pending_flushes);
    }

    /* Prior to sharding the counters were under a global
    mutex. Now they are spread across the shards. Therefore
    it is normal for the modification counter to be out of
    sync with the flush counter for files that are in differnet
    shards. */

    if (file->modification_counter != file->flush_counter) {
      ib::info(ER_IB_MSG_275, file->name, longlong{file->modification_counter},
               longlong{file->flush_counter});
    }

    if (file->in_use > 0) {
      ib::info(ER_IB_MSG_276, file->name);
    }
  }

  return (false);
}

/** Tries to close a file in the LRU list.
@param[in] print_info   if true, prints information why it cannot close a file
@return true if success, false if should retry later */
bool Fil_system::close_file_in_all_LRU(bool print_info) {
  for (auto shard : m_shards) {
    shard->mutex_acquire();

    if (print_info) {
      ib::info(ER_IB_MSG_277, shard->id(),
               ulonglong{UT_LIST_GET_LEN(shard->m_LRU)});
    }

    bool success = shard->close_files_in_LRU(print_info);

    shard->mutex_release();

    if (success) {
      return (true);
    }
  }

  return (false);
}

/** We keep log files and system tablespace files always open; this is
important in preventing deadlocks in this module, as a page read
completion often performs another read from the insert buffer. The
insert buffer is in tablespace TRX_SYS_SPACE, and we cannot end up
waiting in this function.
@param[in]	space_id	Tablespace ID to look up
@return tablespace instance */
fil_space_t *Fil_shard::get_reserved_space(space_id_t space_id) {
  if (space_id == TRX_SYS_SPACE) {
    return (fil_space_t::s_sys_space);

  } else if (space_id == dict_sys_t_s_log_space_first_id &&
             fil_space_t::s_redo_space != nullptr) {
    return (fil_space_t::s_redo_space);
  }

  return (get_space_by_id(space_id));
}

/** Prepare to free a file. Remove from the unflushed list if there
are no pending flushes.
@param[in,out]	file		File instance to free */
void Fil_shard::prepare_to_free_file(fil_node_t *file) {
  ut_ad(mutex_owned());

  fil_space_t *space = file->space;

  if (space->is_in_unflushed_spaces && space_is_flushed(space)) {
    space->is_in_unflushed_spaces = false;

    UT_LIST_REMOVE(m_unflushed_spaces, space);
  }
}

/** Prepare to free a file object from a tablespace memory cache.
@param[in,out]	file	Tablespace file
@param[in]	space	tablespace */
void Fil_shard::file_close_to_free(fil_node_t *file, fil_space_t *space) {
  ut_ad(mutex_owned());
  ut_a(file->magic_n == FIL_NODE_MAGIC_N);
  ut_a(file->n_pending == 0);
  ut_a(file->in_use == 0);
  ut_a(file->space == space);

  if (file->is_open) {
    /* We fool the assertion in Fil_system::close_file() to think
    there are no unflushed modifications in the file */

    file->modification_counter = file->flush_counter;

    os_event_set(file->sync_event);

    if (fil_buffering_disabled(space)) {
      ut_ad(!space->is_in_unflushed_spaces);
      ut_ad(space_is_flushed(space));

    } else {
      prepare_to_free_file(file);
    }

    /* TODO: set second parameter to true, so to release
    fil_system mutex before logging tablespace name and id.
    To go around Bug#26271853 - POTENTIAL DEADLOCK BETWEEN
    FIL_SYSTEM MUTEX AND LOG MUTEX */
    close_file(file, true);
  }
}

/** Detach a space object from the tablespace memory cache.
Closes the tablespace files but does not delete them.
There must not be any pending I/O's or flushes on the files.
@param[in,out]	space		tablespace */
void Fil_shard::space_detach(fil_space_t *space) {
  ut_ad(mutex_owned());

  m_names.erase(space->name);

  if (space->is_in_unflushed_spaces) {
    ut_ad(!fil_buffering_disabled(space));

    space->is_in_unflushed_spaces = false;

    UT_LIST_REMOVE(m_unflushed_spaces, space);
  }

  ut_a(space->magic_n == FIL_SPACE_MAGIC_N);
  ut_a(space->n_pending_flushes == 0);

  for (auto &file : space->files) {
    file_close_to_free(&file, space);
  }
}

/** Explicitly call the destructor, this is to get around Clang bug#12350.
@param[in,out]	p		Instance on which to call the destructor */
template <typename T>
void call_destructor(T *p) {
  p->~T();
}


/** Free a tablespace object on which fil_space_detach() was invoked.
There must not be any pending I/O's or flushes on the files.
@param[in,out]	space		tablespace */
void Fil_shard::space_free_low(fil_space_t *&space) {
  // FIXME
  // ut_ad(srv_fast_shutdown == 2);

  for (auto &file : space->files) {
    ut_d(space->size -= file.size);

    os_event_destroy(file.sync_event);

    ut_free(file.name);
  }

  call_destructor(&space->files);

  ut_ad(space->size == 0);

  rw_lock_free(&space->latch);

  ut_free(space->name);
  ut_free(space);

  space = nullptr;
}

/** Frees a space object from the tablespace memory cache.
Closes a tablespaces' files but does not delete them.
There must not be any pending I/O's or flushes on the files.
@param[in]	space_id	Tablespace ID
@return fil_space_t instance on success or nullptr */
fil_space_t *Fil_shard::space_free(space_id_t space_id) {
  mutex_acquire();

  fil_space_t *space = get_space_by_id(space_id);

  if (space != nullptr) {
    space_detach(space);

    space_delete(space_id);
  }

  mutex_release();

  return (space);
}


/** Opens the files associated with a tablespace and returns a pointer to
the fil_space_t that is in the memory cache associated with a space id.
@param[in]	space_id	Get the tablespace instance or this ID
@return file_space_t pointer, nullptr if space not found */
fil_space_t *Fil_shard::space_load(space_id_t space_id) {
  ut_ad(mutex_owned());

  fil_space_t *space = get_space_by_id(space_id);

  if (space == nullptr || space->size != 0) {
    return (space);
  }

  switch (space->purpose) {
    case FIL_TYPE_LOG:
      break;

    case FIL_TYPE_IMPORT:
    case FIL_TYPE_TEMPORARY:
    case FIL_TYPE_TABLESPACE:

      ut_a(space_id != TRX_SYS_SPACE);

      mutex_release();

      auto slot = mutex_acquire_and_get_space(space_id, space);

      if (space == nullptr) {
        if (slot) {
          release_open_slot(m_id);
        }

        return (nullptr);
      }

      ut_a(1 == space->files.size());

      {
        fil_node_t *file;

        file = &space->files.front();

        /* It must be a single-table tablespace and
        we have not opened the file yet; the following
        calls will open it and update the size fields */

        bool success = prepare_file_for_io(file, false);

        if (slot) {
          release_open_slot(m_id);
        }

        if (!success) {
          /* The single-table tablespace can't be opened,
          because the ibd file is missing. */

          return (nullptr);
        }

        complete_io(file, IORequestRead);
      }
  }

  return (space);
}

/** Open each file of a tablespace if not already open.
@param[in]	space_id	tablespace identifier
@retval	true	if all file nodes were opened
@retval	false	on failure */
bool Fil_shard::space_open(space_id_t space_id) {
  ut_ad(mutex_owned());

  fil_space_t *space = get_space_by_id(space_id);

  for (auto &file : space->files) {
    if (!file.is_open && !open_file(&file, false)) {
      return (false);
    }
  }

  return (true);
}

/** Close all open files. */
void Fil_shard::close_all_files() {
  ut_ad(mutex_owned());

  auto end = m_spaces.end();

  for (auto it = m_spaces.begin(); it != end; it = m_spaces.erase(it)) {
    auto space = it->second;

    ut_a(space->id == TRX_SYS_SPACE || space->purpose == FIL_TYPE_TEMPORARY ||
         space->id == dict_sys_t_s_log_space_first_id ||
         space->files.size() == 1);

    if (space->id == dict_sys_t_s_log_space_first_id) {
      fil_space_t::s_redo_space = nullptr;
    }

    for (auto &file : space->files) {
      if (file.is_open) {
        close_file(&file, false);
      }
    }

    space_detach(space);

    space_free_low(space);

    ut_a(space == nullptr);
  }
}

/** Close all open files. */
void Fil_system::close_all_files() {
  for (auto shard : m_shards) {
    shard->mutex_acquire();

    shard->close_all_files();

    shard->mutex_release();
  }
}

/** Close log files.
@param[in]	free_all	If set then free all instances */
void Fil_shard::close_log_files(bool free_all) {
  mutex_acquire();

  auto end = m_spaces.end();

  for (auto it = m_spaces.begin(); it != end; /* No op */) {
    auto space = it->second;

    if (space->purpose != FIL_TYPE_LOG) {
      ++it;
      continue;
    }

    if (space->id == dict_sys_t_s_log_space_first_id) {
      ut_a(fil_space_t::s_redo_space == space);

      fil_space_t::s_redo_space = nullptr;
    }

    for (auto &file : space->files) {
      if (file.is_open) {
        close_file(&file, false);
      }
    }

    if (free_all) {
      space_detach(space);
      space_free_low(space);
      ut_a(space == nullptr);

      it = m_spaces.erase(it);

    } else {
      ++it;
    }
  }

  mutex_release();
}

/** Close all log files in all shards.
@param[in]	free_all	If set then free all instances */
void Fil_system::close_all_log_files(bool free_all) {
  for (auto shard : m_shards) {
    shard->close_log_files(free_all);
  }
}

/** Iterate through all persistent tablespace files (FIL_TYPE_TABLESPACE)
returning the nodes via callback function cbk.
@param[in]	include_log	Include log files, if true
@param[in]	f		Callback
@return any error returned by the callback function. */
dberr_t Fil_shard::iterate(bool include_log, Fil_iterator::Function &f) {
  mutex_acquire();

  for (auto &elem : m_spaces) {
    auto space = elem.second;

    if (space->purpose != FIL_TYPE_TABLESPACE &&
        (!include_log || space->purpose != FIL_TYPE_LOG)) {
      continue;
    }

    for (auto &file : space->files) {
      /* Note: The callback can release the mutex. */

      dberr_t err = f(&file);

      if (err != DB_SUCCESS) {
        mutex_release();

        return (err);
      }
    }
  }

  mutex_release();

  return (DB_SUCCESS);
}

/** Iterate through all persistent tablespace files (FIL_TYPE_TABLESPACE)
returning the nodes via callback function cbk.
@param[in]	include_log	include log files, if true
@param[in]	f		callback function
@return any error returned by the callback function. */
dberr_t Fil_system::iterate(bool include_log, Fil_iterator::Function &f) {
  for (auto shard : m_shards) {
    dberr_t err = shard->iterate(include_log, f);

    if (err != DB_SUCCESS) {
      return (err);
    }
  }

  return (DB_SUCCESS);
}

/** Check for pending operations.
@param[in]	space	tablespace
@param[in]	count	number of attempts so far
@return 0 if no pending operations else count + 1. */
ulint Fil_shard::space_check_pending_operations(fil_space_t *space,
                                                ulint count) const {
  ut_ad(mutex_owned());

  if (space != nullptr && space->n_pending_ops > 0) {
    if (count > 5000) {
      ib::warn(ER_IB_MSG_287, space->name, ulong{space->n_pending_ops});
    }

    return (count + 1);
  }

  return (0);
}

/** Check for pending IO.
@param[in]	space		Tablespace to check
@param[in]	file		File in space list
@param[in]	count		number of attempts so far
@return 0 if no pending else count + 1. */
ulint Fil_shard::check_pending_io(const fil_space_t *space,
                                  const fil_node_t &file, ulint count) const {
  ut_ad(mutex_owned());
  ut_a(space->n_pending_ops == 0);

  ut_a(space->id == TRX_SYS_SPACE || space->purpose == FIL_TYPE_TEMPORARY ||
       space->id == dict_sys_t_s_log_space_first_id ||
       space->files.size() == 1);

  if (space->n_pending_flushes > 0 || file.n_pending > 0) {
    if (count > 1000) {
      ib::warn(ER_IB_MSG_288, space->name, ulong{space->n_pending_flushes},
               size_t{file.n_pending});
    }

    return (count + 1);
  }

  return (0);
}

/** Check pending operations on a tablespace.
@param[in]	space_id	Tablespace ID
@param[out]	space		tablespace instance in memory
@param[out]	path		tablespace path
@return DB_SUCCESS or DB_TABLESPACE_NOT_FOUND. */
dberr_t Fil_shard::space_check_pending_operations(space_id_t space_id,
                                                  fil_space_t *&space,
                                                  char **path) const {
  ut_ad(!fsp_is_system_tablespace(space_id));
  ut_ad(!fsp_is_global_temporary(space_id));

  space = nullptr;

  mutex_acquire();

  fil_space_t *sp = get_space_by_id(space_id);

  if (sp != nullptr) {
    sp->stop_new_ops = true;
  }

  mutex_release();

  /* Check for pending operations. */

  ulint count = 0;

  do {
    mutex_acquire();

    sp = get_space_by_id(space_id);

    count = space_check_pending_operations(sp, count);

    mutex_release();

    if (count > 0) {
      os_thread_sleep(20000);
    }

  } while (count > 0);

  /* Check for pending IO. */

  *path = 0;

  do {
    mutex_acquire();

    sp = get_space_by_id(space_id);

    if (sp == nullptr) {
      mutex_release();

      return (DB_TABLESPACE_NOT_FOUND);
    }

    const fil_node_t &file = sp->files.front();

    count = check_pending_io(sp, file, count);

    if (count == 0) {
      *path = mem_strdup(file.name);
    }

    mutex_release();

    if (count > 0) {
      os_thread_sleep(20000);
    }

  } while (count > 0);

  ut_ad(sp != nullptr);

  space = sp;

  return (DB_SUCCESS);
}




/** Try to extend a tablespace if it is smaller than the specified size.
@param[in,out]	space		tablespace
@param[in]	size		desired size in pages
@return whether the tablespace is at least as big as requested */
bool Fil_shard::space_extend(fil_space_t *space, page_no_t size) {
  /* In read-only mode we allow write to shared temporary tablespace
  as intrinsic table created by Optimizer reside in this tablespace. */
  ut_ad(!srv_read_only_mode || fsp_is_system_temporary(space->id));

#ifndef UNIV_HOTBACKUP
  DBUG_EXECUTE_IF("fil_space_print_xdes_pages",
                  space->print_xdes_pages("xdes_pages.log"););
#endif /* !UNIV_HOTBACKUP */

  fil_node_t *file;
  bool slot;
  size_t phy_page_size;
  bool success = true;

#ifdef UNIV_HOTBACKUP
  page_no_t prev_size = 0;
#endif /* UNIV_HOTBACKUP */

  for (;;) {
    slot = mutex_acquire_and_get_space(space->id, space);

    /* Note:If the file is being opened for the first time then
    we don't have the file physical size. There is no guarantee
    that the file has been opened at this stage. */

    if (size < space->size) {
      /* Space already big enough */
      mutex_release();

      if (slot) {
        release_open_slot(m_id);
      }

      return (true);
    }

    file = &space->files.back();

    page_size_t page_size(space->flags);

    phy_page_size = page_size.physical();

#ifdef UNIV_HOTBACKUP
    prev_size = space->size;

    ib::trace_1() << "Extending space id : " << space->id
                  << ", space name : " << space->name
                  << ", space size : " << space->size
                  << " page, page size : " << phy_page_size;
#endif /* UNIV_HOTBACKUP */

    if (file->in_use == 0) {
      /* Mark this file as undergoing extension. This flag
      is used by other threads to wait for the extension
      opereation to finish or wait for open to complete. */

      ++file->in_use;

      break;
    }

    if (slot) {
      release_open_slot(m_id);
    }

    /* Another thread is currently using the file. Wait
    for it to finish.  It'd have been better to use an event
    driven mechanism but the entire module is peppered with
    polling code. */

    mutex_release();

    os_thread_sleep(100000);
  }

  bool opened = prepare_file_for_io(file, true);

  if (slot) {
    release_open_slot(m_id);
  }

  if (!opened) {
    /* The tablespace data file, such as .ibd file, is missing */
    ut_a(file->in_use > 0);
    --file->in_use;

    mutex_release();

    return (false);
  }

  ut_a(file->is_open);

  if (size <= space->size) {
    ut_a(file->in_use > 0);
    --file->in_use;

    complete_io(file, IORequestRead);

    mutex_release();

    return (true);
  }

  /* At this point it is safe to release the shard mutex. No
  other thread can rename, delete or close the file because
  we have set the file->in_use flag. */

  mutex_release();

  page_no_t pages_added;
  os_offset_t node_start = os_file_get_size(file->handle);

  ut_a(node_start != (os_offset_t)-1);

  /* File first page number */
  page_no_t node_first_page = space->size - file->size;

  /* Number of physical pages in the file */
  page_no_t n_node_physical_pages =
      static_cast<page_no_t>(node_start / phy_page_size);

  /* Number of pages to extend in the file */
  page_no_t n_node_extend;

  n_node_extend = size - (node_first_page + file->size);

  /* If we already have enough physical pages to satisfy the
  extend request on the file then ignore it */
  if (file->size + n_node_extend > n_node_physical_pages) {
    DBUG_EXECUTE_IF("ib_crash_during_tablespace_extension", DBUG_SUICIDE(););

    os_offset_t len;
    dberr_t err = DB_SUCCESS;

    len = ((file->size + n_node_extend) * phy_page_size) - node_start;

    ut_ad(len > 0);

#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)
    /* This is required by FusionIO HW/Firmware */

    int ret = posix_fallocate(file->handle.m_file, node_start, len);

    DBUG_EXECUTE_IF("ib_posix_fallocate_fail_eintr", ret = EINTR;);

    DBUG_EXECUTE_IF("ib_posix_fallocate_fail_einval", ret = EINVAL;);

    if (ret != 0) {
      /* We already pass the valid offset and len in, if EINVAL
      is returned, it could only mean that the file system doesn't
      support fallocate(), currently one known case is ext3 with O_DIRECT.

      Also because above call could be interrupted, in this case,
      simply go to plan B by writing zeroes.

      Both error messages for above two scenarios are skipped in case
      of flooding error messages, because they can be ignored by users. */
      if (ret != EINTR && ret != EINVAL) {
        ib::error(ER_IB_MSG_319)
            << "posix_fallocate(): Failed to preallocate"
               " data for file "
            << file->name << ", desired size " << len
            << " bytes."
               " Operating system error number "
            << ret
            << ". Check"
               " that the disk is not full or a disk quota"
               " exceeded. Make sure the file system supports"
               " this function. Some operating system error"
               " numbers are described at " REFMAN
               "operating-system-error-codes.html";
      }

      err = DB_IO_ERROR;
    }
#endif /* NO_FALLOCATE || !UNIV_LINUX */

    if (!file->atomic_write || err == DB_IO_ERROR) {
      bool read_only_mode;

      read_only_mode =
          (space->purpose != FIL_TYPE_TEMPORARY ? false : srv_read_only_mode);

      err = fil_write_zeros(file, phy_page_size, node_start,
                            static_cast<ulint>(len), read_only_mode);

      if (err != DB_SUCCESS) {
        ib::warn(ER_IB_MSG_320)
            << "Error while writing " << len << " zeroes to " << file->name
            << " starting at offset " << node_start;
      }
    }

    /* Check how many pages actually added */
    os_offset_t end = os_file_get_size(file->handle);
    ut_a(end != static_cast<os_offset_t>(-1) && end >= node_start);

    os_has_said_disk_full = !(success = (end == node_start + len));

    pages_added = static_cast<page_no_t>(end / phy_page_size);

    ut_a(pages_added >= file->size);
    pages_added -= file->size;

  } else {
    success = true;
    pages_added = n_node_extend;
    os_has_said_disk_full = FALSE;
  }

  mutex_acquire();

  file->size += pages_added;
  space->size += pages_added;

  ut_a(file->in_use > 0);
  --file->in_use;

  complete_io(file, IORequestWrite);

#ifndef UNIV_HOTBACKUP
  /* Keep the last data file size info up to date, rounded to
  full megabytes */
  page_no_t pages_per_mb =
      static_cast<page_no_t>((1024 * 1024) / phy_page_size);

  page_no_t size_in_pages = ((file->size / pages_per_mb) * pages_per_mb);

  if (space->id == TRX_SYS_SPACE) {
    srv_sys_space.set_last_file_size(size_in_pages);
  } else if (fsp_is_system_temporary(space->id)) {
    srv_tmp_space.set_last_file_size(size_in_pages);
  }
#else  /* !UNIV_HOTBACKUP */
  ib::trace_2() << "Extended space : " << space->name << " from " << prev_size
                << " pages to " << space->size << " pages "
                << ", desired space size : " << size << " pages.";
#endif /* !UNIV_HOTBACKUP */

  space_flush(space->id);

  mutex_release();

  return (success);
}

/** We are going to do a rename file and want to stop new I/O for a while.
@param[in]	space		Tablespace for which we want to wait for IO
                                to stop */
void Fil_shard::wait_for_io_to_stop(const fil_space_t *space) {
  /* Note: We are reading the value of space->stop_ios without the
  cover of the Fil_shard::mutex. We incremented the in_use counter
  before waiting for IO to stop. */

  auto begin_time = ut_time();
  auto start_time = begin_time;

  /* Spam the log after every minute. Ignore any race here. */

  while (space->stop_ios) {
    if ((ut_time() - start_time) == PRINT_INTERVAL_SECS) {
      start_time = ut_time();

      ib::warn(ER_IB_MSG_278, space->name, longlong{ut_time() - begin_time});
    }

#ifndef UNIV_HOTBACKUP

    /* Wake the I/O handler threads to make sure
    pending I/O's are performed */
    os_aio_simulated_wake_handler_threads();

#endif /* UNIV_HOTBACKUP */

    /* Give the IO threads some time to work. */
    os_thread_yield();
  }
}

/** Reserves the mutex and tries to make sure we can open at least
one file while holding it. This should be called before calling
prepare_file_for_io(), because that function may need to open a file.
@param[in]	space_id	Tablespace ID
@param[out]	space		Tablespace instance
@return true if a slot was reserved. */
bool Fil_shard::mutex_acquire_and_get_space(space_id_t space_id,
                                            fil_space_t *&space) {
  mutex_acquire();

  if (space_id == TRX_SYS_SPACE || dict_sys_t_is_reserved(space_id)) {
    space = get_reserved_space(space_id);

    return (false);
  }

  space = get_space_by_id(space_id);

  if (space == nullptr) {
    /* Caller handles the case of a missing tablespce. */
    return (false);
  }

  ut_ad(space->files.size() == 1);

  auto is_open = space->files.front().is_open;

  if (is_open) {
    /* Ensure that the file is not closed behind our back. */
    ++space->files.front().in_use;
  }

  mutex_release();

  if (is_open) {
    wait_for_io_to_stop(space);

    mutex_acquire();

    /* We are guaranteed that this file cannot be closed
    because we now own the mutex. */

    ut_ad(space->files.front().in_use > 0);
    --space->files.front().in_use;

    return (false);
  }

  /* The number of open file descriptors is a shared resource, in
  order to guarantee that we don't over commit, we use a ticket system
  to reserve a slot/ticket to open a file. This slot/ticket should
  be released after the file is opened. */

  while (!reserve_open_slot(m_id)) {
    os_thread_yield();
  }

  auto begin_time = ut_time();
  auto start_time = begin_time;

  for (size_t i = 0; i < 3; ++i) {
    /* Flush tablespaces so that we can close modified
    files in the LRU list */

    auto type = to_int(FIL_TYPE_TABLESPACE);

    fil_system->flush_file_spaces(type);

    os_thread_yield();

    /* Reserve an open slot for this shard. So that this
    shard's open file succeeds. */

    while (fil_system->m_max_n_open <= s_n_open &&
           !fil_system->close_file_in_all_LRU(i > 1)) {
      if (ut_time() - start_time == PRINT_INTERVAL_SECS) {
        start_time = ut_time();

        ib::warn(ER_IB_MSG_279) << "Trying to close a file for "
                                << ut_time() - begin_time << " seconds"
                                << ". Configuration only allows for "
                                << fil_system->m_max_n_open << " open files.";
      }
    }

    if (fil_system->m_max_n_open > s_n_open) {
      break;
    }

#ifndef UNIV_HOTBACKUP
    /* Wake the I/O-handler threads to make sure pending I/Os are
    performed */
    os_aio_simulated_wake_handler_threads();

    os_thread_yield();
#endif /* !UNIV_HOTBACKUP */
  }

#if 0
    /* The magic value of 300 comes from innodb.open_file_lru.test */
    if (fil_system->m_max_n_open == 300) {
        ib::warn(ER_IB_MSG_280)
            << "Too many (" << s_n_open
            << ") files are open the maximum allowed"
            << " value is " << fil_system->m_max_n_open
            << ". You should raise the value of"
            << " --innodb-open-files in my.cnf.";
    }
#endif

  mutex_acquire();

  return (true);
}



/** If the tablespace is on the unflushed list and there are no pending
flushes then remove from the unflushed list.
@param[in,out]	space		Tablespace to remove */
void Fil_shard::remove_from_unflushed_list(fil_space_t *space) {
  ut_ad(mutex_owned());

  if (space->is_in_unflushed_spaces && space_is_flushed(space)) {
    space->is_in_unflushed_spaces = false;

    UT_LIST_REMOVE(m_unflushed_spaces, space);
  }
}

/** Flushes to disk possible writes cached by the OS. */
void Fil_shard::redo_space_flush() {
  ut_ad(mutex_owned());
  ut_ad(m_id == REDO_SHARD);

  fil_space_t *space = fil_space_t::s_redo_space;

  if (space == nullptr) {
    space = get_space_by_id(dict_sys_t_s_log_space_first_id);
  } else {
    ut_ad(space == get_space_by_id(dict_sys_t_s_log_space_first_id));
  }

  ut_a(!space->stop_new_ops);
  ut_a(space->purpose == FIL_TYPE_LOG);

  /* Prevent dropping of the space while we are flushing */
  ++space->n_pending_flushes;

  for (auto &file : space->files) {
    ut_a(!file.is_raw_disk);

    int64_t old_mod_counter = file.modification_counter;

    if (old_mod_counter <= file.flush_counter) {
      continue;
    }

    ut_a(file.is_open);
    ut_a(file.space == space);

    ++fil_n_log_flushes;
    ++fil_n_pending_log_flushes;

    bool skip_flush = false;

    /* Wait for some other thread that is flushing. */
    while (file.n_pending_flushes > 0 && !skip_flush) {
      /* Release the mutex to avoid deadlock with
      the flushing thread. */

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

      mutex_acquire();

      os_event_set(file.sync_event);

      --file.n_pending_flushes;
    }

    if (file.flush_counter < old_mod_counter) {
      file.flush_counter = old_mod_counter;

      remove_from_unflushed_list(space);
    }

    --fil_n_pending_log_flushes;
  }

  --space->n_pending_flushes;
}


/** Create a space memory object and put it to the fil_system hash table.
The tablespace name is independent from the tablespace file-name.
Error messages are issued to the server log.
@param[in]	name		Tablespace name
@param[in]	space_id	Tablespace identifier
@param[in]	flags		Tablespace flags
@param[in]	purpose		Tablespace purpose
@return pointer to created tablespace, to be filled in with fil_node_create()
@retval nullptr on failure (such as when the same tablespace exists) */
fil_space_t *Fil_shard::space_create(const char *name, space_id_t space_id,
                                     uint32_t flags, fil_type_t purpose) {
  ut_ad(mutex_owned());

  /* Look for a matching tablespace. */
  fil_space_t *space = get_space_by_name(name);

  if (space == nullptr) {
    space = get_space_by_id(space_id);
  }

  if (space != nullptr) {
    std::ostringstream oss;

    for (size_t i = 0; i < space->files.size(); ++i) {
      oss << "'" << space->files[i].name << "'";

      if (i < space->files.size() - 1) {
        oss << ", ";
      }
    }

    ib::info(ER_IB_MSG_281)
        << "Trying to add tablespace '" << name << "'"
        << " with id " << space_id << " to the tablespace"
        << " memory cache, but tablespace"
        << " '" << space->name << "'"
        << " already exists in the cache with space ID " << space->id
        << ". It maps to the following file(s): " << oss.str();

    return (nullptr);
  }

  space = static_cast<fil_space_t *>(ut_zalloc_nokey(sizeof(*space)));

  space->id = space_id;

  space->name = mem_strdup(name);

  new (&space->files) fil_space_t::Files();

#ifndef UNIV_HOTBACKUP
  if (fil_system->is_greater_than_max_id(space_id) &&
      fil_type_is_data(purpose) && !recv_recovery_on &&
      !dict_sys_t_is_reserved(space_id) &&
      !fsp_is_system_temporary(space_id)) {
    fil_system->set_maximum_space_id(space);
  }
#endif /* !UNIV_HOTBACKUP */

  space->purpose = purpose;

  ut_a(flags < std::numeric_limits<uint32_t>::max());
  space->flags = (uint32_t)flags;

  space->magic_n = FIL_SPACE_MAGIC_N;

  space->encryption_type = Encryption::NONE;

  rw_lock_create(fil_space_latch_key, &space->latch, SYNC_FSP);

#ifndef UNIV_HOTBACKUP
  if (space->purpose == FIL_TYPE_TEMPORARY) {
    ut_d(space->latch.set_temp_fsp());
  }
#endif /* !UNIV_HOTBACKUP */

  space_add(space);

  return (space);
}

/** Open all the system files.
@param[in]	max_n_open	Maximum number of open files allowed
@param[in,out]	n_open		Current number of open files */
void Fil_shard::open_system_tablespaces(size_t max_n_open, size_t *n_open) {
  mutex_acquire();

  for (auto elem : m_spaces) {
    auto space = elem.second;

    if (Fil_system::space_belongs_in_LRU(space)) {
      continue;
    }

    for (auto &file : space->files) {
      if (!file.is_open) {
        if (!open_file(&file, false)) {
          /* This func is called during server's
          startup. If some file of log or system
          tablespace is missing, the server
          can't start successfully. So we should
          assert for it. */
          ut_a(0);
        }

        ++*n_open;
      }

      if (max_n_open < 10 + *n_open) {
        ib::warn(ER_IB_MSG_284, *n_open, max_n_open);
      }
    }
  }

  mutex_release();
}


/** Prepare for truncating a single-table tablespace.
1) Check pending operations on a tablespace;
2) Remove all insert buffer entries for the tablespace;
@param[in]	space_id	Tablespace ID
@return DB_SUCCESS or error */
dberr_t Fil_shard::space_prepare_for_truncate(space_id_t space_id) {
  char *path = nullptr;
  fil_space_t *space = nullptr;

  ut_ad(space_id != TRX_SYS_SPACE);
  ut_ad(!fsp_is_system_tablespace(space_id));
  ut_ad(!fsp_is_global_temporary(space_id));
  ut_ad(fsp_is_undo_tablespace(space_id) || fsp_is_session_temporary(space_id));

  dberr_t err = space_check_pending_operations(space_id, space, &path);

  ut_free(path);

  return (err);
}

/** Prepares a file for I/O. Opens the file if it is closed. Updates the
pending I/O's field in the file and the system appropriately. Takes the file
off the LRU list if it is in the LRU list.
@param[in]	file		Tablespace file
@param[in]	extend		true if file is being extended
@return false if the file can't be opened, otherwise true */
bool Fil_shard::prepare_file_for_io(fil_node_t *file, bool extend) {
  ut_ad(mutex_owned());

  fil_space_t *space = file->space;

  if (s_n_open > fil_system->m_max_n_open + 5) {
    static ulint prev_time;
    auto curr_time = ut_time();

    /* Spam the log after every minute. Ignore any race here. */

    if ((curr_time - prev_time) > 60) {
      ib::warn(ER_IB_MSG_327)
          << "Open files " << s_n_open.load() << " exceeds the limit "
          << fil_system->m_max_n_open;

      prev_time = curr_time;
    }
  }

  if (!file->is_open) {
    ut_a(file->n_pending == 0);

    if (!open_file(file, extend)) {
      return (false);
    }
  }

  if (file->n_pending == 0 && Fil_system::space_belongs_in_LRU(space)) {
    /* The file is in the LRU list, remove it */

    ut_a(UT_LIST_GET_LEN(m_LRU) > 0);

    UT_LIST_REMOVE(m_LRU, file);
  }

  ++file->n_pending;

  return (true);
}

/** If the tablespace is not on the unflushed list, add it.
@param[in,out]	space		Tablespace to add */
void Fil_shard::add_to_unflushed_list(fil_space_t *space) {
  ut_ad(m_id == REDO_SHARD || mutex_owned());

  if (!space->is_in_unflushed_spaces) {
    space->is_in_unflushed_spaces = true;

    UT_LIST_ADD_FIRST(m_unflushed_spaces, space);
  }
}

/** Note that a write IO has completed.
@param[in,out]	file		File on which a write was completed */
void Fil_shard::write_completed(fil_node_t *file) {
  ut_ad(m_id == REDO_SHARD || mutex_owned());

  ++m_modification_counter;

  file->modification_counter = m_modification_counter;

  if (fil_buffering_disabled(file->space)) {
    /* We don't need to keep track of unflushed
    changes as user has explicitly disabled
    buffering. */
    ut_ad(!file->space->is_in_unflushed_spaces);

    file->flush_counter = file->modification_counter;

  } else {
    add_to_unflushed_list(file->space);
  }
}

/** Updates the data structures when an I/O operation finishes. Updates the
pending i/o's field in the file appropriately.
@param[in]	file		Tablespace file
@param[in]	type		Marks the file as modified if type == WRITE */
void Fil_shard::complete_io(fil_node_t *file, const IORequest &type) {
  ut_ad(m_id == REDO_SHARD || mutex_owned());

  ut_a(file->n_pending > 0);

  --file->n_pending;

  ut_ad(type.validate());

  if (type.is_write()) {
    ut_ad(!srv_read_only_mode || fsp_is_system_temporary(file->space->id));

    write_completed(file);
  }

  if (file->n_pending == 0 && Fil_system::space_belongs_in_LRU(file->space)) {
    /* The file must be put back to the LRU list */
    UT_LIST_ADD_FIRST(m_LRU, file);
  }
}

/** Get the AIO mode.
@param[in]	req_type	IO request type
@param[in]	sync		true if Synchronous IO
return the AIO mode */
AIO_mode Fil_shard::get_AIO_mode(const IORequest &req_type, bool sync) {
#ifndef UNIV_HOTBACKUP
  if (sync) {
    return (AIO_mode::SYNC);

  } else if (req_type.is_log()) {
    return (AIO_mode::LOG);

  } else {
    return (AIO_mode::NORMAL);
  }
#else  /* !UNIV_HOTBACKUP */
  ut_a(sync);
  return (AIO_mode::SYNC);
#endif /* !UNIV_HOTBACKUP */
}


