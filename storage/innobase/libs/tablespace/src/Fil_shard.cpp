#include <innodb/tablespace/Fil_shard.h>

#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/error/ut_error.h>
#include <innodb/formatting/hex.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/access_type.h>
#include <innodb/io/innodb_data_file_key.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_create_simple_no_error_handling.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/os_file_get_last_error.h>
#include <innodb/io/os_file_get_size.h>
#include <innodb/io/os_file_get_status.h>
#include <innodb/io/os_file_stat_t.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/logger/fatal.h>
#include <innodb/math/ut_2pow_round.h>
#include <innodb/string/mem_strdup.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/tablespace/Fil_system.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/extent.h>
#include <innodb/tablespace/fil_n_file_opened.h>
#include <innodb/tablespace/fil_no_punch_hole.h>
#include <innodb/tablespace/header.h>
#include <innodb/tablespace/srv_start_raw_disk_in_use.h>



#include <atomic>

bool fsp_is_system_temporary(space_id_t space_id);
bool recv_recovery_is_on();

/** Total number of open files. */
std::atomic_size_t Fil_shard::s_n_open;

/** Slot reserved for opening a file. */
std::atomic_size_t Fil_shard::s_open_slot;

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



