#include <innodb/tablespace/Fil_system.h>

#include <innodb/tablespace/recv_recovery_on.h>
#include <innodb/tablespace/Datafile.h>
#include <innodb/tablespace/fsp_is_ibd_tablespace.h>
#include <innodb/machine/data.h>
#include <innodb/tablespace/dict_sys_t_is_reserved.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fil_system.h>
#include <innodb/io/IORequestRead.h>
#include <innodb/io/os_file_get_size.h>



#include <fstream>

/** Constructor.
@param[in]	n_shards	Number of shards to create
@param[in]	max_open	Maximum number of open files */
Fil_system::Fil_system(size_t n_shards, size_t max_open)
    : m_shards(),
      m_max_n_open(max_open),
      m_max_assigned_id(),
      m_space_id_reuse_warned() {
  ut_ad(Fil_shard::s_open_slot == 0);
  Fil_shard::s_open_slot = EMPTY_OPEN_SLOT;

  for (size_t i = 0; i < n_shards; ++i) {
    auto shard = UT_NEW_NOKEY(Fil_shard(i));

    m_shards.push_back(shard);
  }
}

/** Destructor */
Fil_system::~Fil_system() {
  ut_ad(Fil_shard::s_open_slot == EMPTY_OPEN_SLOT);

  Fil_shard::s_open_slot = 0;

  for (auto shard : m_shards) {
    UT_DELETE(shard);
  }

  m_shards.clear();
}

/** Opens all log files and system tablespace data files in all shards. */
void Fil_system::open_all_system_tablespaces() {
  size_t n_open = 0;

  for (auto shard : m_shards) {
    shard->open_system_tablespaces(m_max_n_open, &n_open);
  }
}

/** Flush the redo log writes to disk, possibly cached by the OS. */
void Fil_system::flush_file_redo() {
    m_shards[REDO_SHARD]->flush_file_redo();
}

/** Flush to disk the writes in file spaces of the given type
possibly cached by the OS.
@param[in]	purpose		FIL_TYPE_TABLESPACE or FIL_TYPE_LOG,
                                can be ORred */
void Fil_system::flush_file_spaces(uint8_t purpose) {
  for (auto shard : m_shards) {
    shard->flush_file_spaces(purpose);
  }
}

/** Rotate the tablespace keys by new master key.
@return true if the re-encrypt succeeds */
bool Fil_system::encryption_rotate_all() {
  for (auto shard : m_shards) {
    // FIXME: We don't acquire the fil_sys::mutex here. Why?

    bool success = encryption_rotate_in_a_shard(shard);

    if (!success) {
      return (false);
    }
  }

  return (true);
}

/** Get the tablespace ID from an .ibd and/or an undo tablespace. If the ID
is == 0 on the first page then check for at least MAX_PAGES_TO_CHECK  pages
with the same tablespace ID. Do a Light weight check before trying with
Datafile::find_space_id().
@param[in]	filename	File name to check
@return s_invalid_space_id if not found, otherwise the space ID */
space_id_t Fil_system::get_tablespace_id(const std::string &filename) {
  char buf[sizeof(space_id_t)];
  std::ifstream ifs(filename, std::ios::binary);

  if (!ifs) {
    ib::warn(ER_IB_MSG_372) << "Unable to open '" << filename << "'";
    return (dict_sys_t_s_invalid_space_id);
  }

  std::vector<space_id_t> space_ids;
  auto page_size = srv_page_size;

  space_ids.reserve(MAX_PAGES_TO_CHECK);

  for (page_no_t page_no = 0; page_no < MAX_PAGES_TO_CHECK; ++page_no) {
    off_t off;

    off = page_no * page_size + FIL_PAGE_SPACE_ID;

    if (off == FIL_PAGE_SPACE_ID) {
      /* Figure out the page size of the tablespace. If it's
      a compressed tablespace. */
      ifs.seekg(FSP_HEADER_OFFSET + FSP_SPACE_FLAGS, ifs.beg);

      if ((ifs.rdstate() & std::ifstream::eofbit) != 0 ||
          (ifs.rdstate() & std::ifstream::failbit) != 0 ||
          (ifs.rdstate() & std::ifstream::badbit) != 0) {
        return (dict_sys_t_s_invalid_space_id);
      }

      ifs.read(buf, sizeof(buf));

      if (!ifs.good() || (size_t)ifs.gcount() < sizeof(buf)) {
        return (dict_sys_t_s_invalid_space_id);
      }

      uint32_t flags;

      flags = mach_read_from_4(reinterpret_cast<byte *>(buf));

      const page_size_t space_page_size(flags);

      page_size = space_page_size.physical();
    }

    ifs.seekg(off, ifs.beg);

    if ((ifs.rdstate() & std::ifstream::eofbit) != 0 ||
        (ifs.rdstate() & std::ifstream::failbit) != 0 ||
        (ifs.rdstate() & std::ifstream::badbit) != 0) {
      /* Trucated files can be a single page */
      break;
    }

    ifs.read(buf, sizeof(buf));

    if (!ifs.good() || (size_t)ifs.gcount() < sizeof(buf)) {
      /* Trucated files can be a single page */
      break;
    }

    space_id_t space_id;

    space_id = mach_read_from_4(reinterpret_cast<byte *>(buf));

    space_ids.push_back(space_id);
  }

  ifs.close();

  space_id_t space_id;

  if (!space_ids.empty()) {
    space_id = space_ids.front();

    for (auto id : space_ids) {
      if (id == 0 || space_id != id) {
        space_id = UINT32_UNDEFINED;

        break;
      }
    }
  } else {
    space_id = UINT32_UNDEFINED;
  }

  /* Try the more heavy duty method, as a last resort. */
  if (space_id == UINT32_UNDEFINED) {
    /* The ifstream will work for all file formats compressed or
    otherwise because the header of the page is not compressed.
    Where it will fail is if the first page is corrupt. Then for
    compressed tablespaces we don't know where the page boundary
    starts because we don't know the page size. */

    Datafile file;

    file.set_filepath(filename.c_str());

    dberr_t err = file.open_read_only(false);

    ut_a(file.is_open());
    ut_a(err == DB_SUCCESS);

    /* Read and validate the first page of the tablespace.
    Assign a tablespace name based on the tablespace type. */
    err = file.find_space_id();

    if (err == DB_SUCCESS) {
      space_id = file.space_id();
    }

    file.close();
  }

  return (space_id);
}

/** Determines if a file belongs to the least-recently-used list.
@param[in]	space		Tablespace to check
@return true if the file belongs to m_LRU. */
bool Fil_system::space_belongs_in_LRU(const fil_space_t *space) {
  switch (space->purpose) {
    case FIL_TYPE_TEMPORARY:
    case FIL_TYPE_LOG:
      return (false);

    case FIL_TYPE_TABLESPACE:
      return (fsp_is_ibd_tablespace(space->id));

    case FIL_TYPE_IMPORT:
      return (true);
  }

  ut_ad(0);
  return (false);
}

/** Assigns a new space id for a new single-table tablespace. This works
simply by incrementing the global counter. If 4 billion id's is not enough,
we may need to recycle id's.
@param[out]	space_id		Set this to the new tablespace ID
@return true if assigned, false if not */
bool Fil_system::assign_new_space_id(space_id_t *space_id) {
  mutex_acquire_all();

  space_id_t id = *space_id;

  if (id < m_max_assigned_id) {
    id = m_max_assigned_id;
  }

  ++id;

  space_id_t reserved_space_id = dict_sys_t_s_reserved_space_id;

  if (id > (reserved_space_id / 2) && (id % 1000000UL == 0)) {
    ib::warn(ER_IB_MSG_282)
        << "You are running out of new single-table"
           " tablespace id's. Current counter is "
        << id << " and it must not exceed " << reserved_space_id
        << "! To reset the counter to zero you have to dump"
           " all your tables and recreate the whole InnoDB"
           " installation.";
  }

  bool success = !dict_sys_t_is_reserved(id);

  if (success) {
    *space_id = m_max_assigned_id = id;

  } else {
    ib::warn(ER_IB_MSG_283) << "You have run out of single-table tablespace"
                               " id's! Current counter is "
                            << id
                            << ". To reset the counter to zero"
                               " you have to dump all your tables and"
                               " recreate the whole InnoDB installation.";

    *space_id = SPACE_UNKNOWN;
  }

  mutex_release_all();

  return (success);
}

/** Rename a tablespace.  Use the space_id to find the shard.
@param[in]	space_id	tablespace ID
@param[in]	old_name	old tablespace name
@param[in]	new_name	new tablespace name
@return DB_SUCCESS on success */
dberr_t Fil_system::rename_tablespace_name(space_id_t space_id,
                                           const char *old_name,
                                           const char *new_name) {
  Fil_shard *old_shard = fil_system->shard_by_id(space_id);

  old_shard->mutex_acquire();

  fil_space_t *old_space = old_shard->get_space_by_id(space_id);

  if (old_space == nullptr) {
    ib::error(ER_IB_MSG_299, old_name);

    return (DB_TABLESPACE_NOT_FOUND);
  }

  ut_ad(old_space == old_shard->get_space_by_name(old_name));

  old_shard->mutex_release();

  Fil_shard *new_shard = nullptr;
  fil_space_t *new_space = nullptr;

  mutex_acquire_all();

  for (auto shard : m_shards) {
    new_space = shard->get_space_by_name(new_name);

    if (new_space != nullptr) {
      new_shard = shard;
      break;
    }
  }

  if (new_space != nullptr) {
    mutex_release_all();

    if (new_space->id != old_space->id) {
      ib::error(ER_IB_MSG_300, new_name);

      return (DB_TABLESPACE_EXISTS);
    } else {
      ut_a(new_shard == old_shard);
    }

    return (DB_SUCCESS);
  }

  auto new_space_name = mem_strdup(new_name);
  auto old_space_name = old_space->name;

  old_shard->update_space_name_map(old_space, new_space_name);

  old_space->name = new_space_name;

  mutex_release_all();

  ut_free(old_space_name);

  return (DB_SUCCESS);
}


