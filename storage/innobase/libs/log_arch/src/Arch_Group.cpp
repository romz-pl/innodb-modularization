#include <innodb/log_arch/Arch_Group.h>

#include <innodb/allocator/ut_malloc.h>
#include <innodb/allocator/ut_zalloc.h>
#include <innodb/io/create_purpose.h>
#include <innodb/io/create_type.h>
#include <innodb/io/os_file_create.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/os_file_delete.h>
#include <innodb/io/os_file_status.h>
#include <innodb/io/os_file_type_t.h>
#include <innodb/io/pfs.h>

Arch_File_Ctx Arch_Group::s_dblwr_file_ctx;

dberr_t Arch_Group::write_to_file(Arch_File_Ctx *from_file, byte *from_buffer,
                                  uint length, bool partial_write,
                                  bool do_persist) {
  dberr_t err = DB_SUCCESS;
  uint write_size;

  if (m_file_ctx.is_closed()) {
    /* First file in the archive group. */
    ut_ad(m_file_ctx.get_count() == 0);

    DBUG_EXECUTE_IF("crash_before_archive_file_creation", DBUG_SUICIDE(););
    err = m_file_ctx.open_new(m_begin_lsn, m_header_len);

    if (err != DB_SUCCESS) {
      return (err);
    }
  }

  auto len_left = m_file_ctx.bytes_left();

  /* New file is immediately opened when current file is over. */
  ut_ad(len_left != 0);

  while (length > 0) {
    auto len_copy = static_cast<uint64_t>(length);

    /* Write as much as possible in current file. */
    if (len_left < len_copy) {
      write_size = static_cast<uint>(len_left);
    } else {
      write_size = length;
    }

    if (do_persist) {
      Arch_Page_Dblwr_Offset dblwr_offset =
          (partial_write ? ARCH_PAGE_DBLWR_PARTIAL_FLUSH_PAGE
                         : ARCH_PAGE_DBLWR_FULL_FLUSH_PAGE);

      Arch_Group::write_to_doublewrite_file(from_file, from_buffer, write_size,
                                            dblwr_offset);
    }

    if (partial_write) {
      DBUG_EXECUTE_IF("crash_after_partial_block_dblwr_flush", DBUG_SUICIDE(););
      err = m_file_ctx.write(from_file, from_buffer, m_file_ctx.get_offset(),
                             write_size);
    } else {
      DBUG_EXECUTE_IF("crash_after_full_block_dblwr_flush", DBUG_SUICIDE(););
      err = m_file_ctx.write(from_file, from_buffer, write_size);
    }

    if (err != DB_SUCCESS) {
      return (err);
    }

    if (do_persist) {
      /* Flush the file to make sure the changes are made persistent as there
      would be no way to recover the data otherwise in case of a crash. */
      m_file_ctx.flush();
    }

    ut_ad(length >= write_size);
    length -= write_size;

    len_left = m_file_ctx.bytes_left();

    /* Current file is over, switch to next file. */
    if (len_left == 0) {
      m_file_ctx.close();

      err = m_file_ctx.open_new(m_begin_lsn, m_header_len);
      DBUG_EXECUTE_IF("crash_after_archive_file_creation", DBUG_SUICIDE(););

      if (err != DB_SUCCESS) {
        return (err);
      }

      len_left = m_file_ctx.bytes_left();
    }
  }

  return (DB_SUCCESS);
}


bool Arch_File_Ctx::delete_file(uint file_index, lsn_t begin_lsn) {
  bool success;
  char file_name[MAX_ARCH_PAGE_FILE_NAME_LEN];

  build_name(file_index, begin_lsn, file_name, MAX_ARCH_PAGE_FILE_NAME_LEN);

  os_file_type_t type;
  bool exists = false;

  success = os_file_status(file_name, &exists, &type);

  if (!success || !exists) {
    return (false);
  }

  ut_ad(type == OS_FILE_TYPE_FILE);

  success = os_file_delete(innodb_arch_file_key, file_name);

  return (success);
}

Arch_Group::~Arch_Group() {
  ut_ad(!m_is_active);

  m_file_ctx.close();

  if (m_active_file.m_file != OS_FILE_CLOSED) {
    os_file_close(m_active_file);
  }

  if (m_durable_file.m_file != OS_FILE_CLOSED) {
    os_file_close(m_durable_file);
  }

  if (m_active_file_name != nullptr) {
    ut_free(m_active_file_name);
  }

  if (m_durable_file_name != nullptr) {
    ut_free(m_durable_file_name);
  }

  if (!is_durable()) {
    m_file_ctx.delete_files(m_begin_lsn);
  }
}

dberr_t Arch_Group::write_to_doublewrite_file(Arch_File_Ctx *from_file,
                                              byte *from_buffer,
                                              uint write_size,
                                              Arch_Page_Dblwr_Offset offset) {
  dberr_t err = DB_SUCCESS;

  ut_ad(!s_dblwr_file_ctx.is_closed());

  switch (offset) {
    case ARCH_PAGE_DBLWR_RESET_PAGE:
      DBUG_EXECUTE_IF("crash_before_reset_block_dblwr_flush", DBUG_SUICIDE(););
      break;

    case ARCH_PAGE_DBLWR_PARTIAL_FLUSH_PAGE:
      DBUG_EXECUTE_IF("crash_before_partial_block_dblwr_flush",
                      DBUG_SUICIDE(););
      break;

    case ARCH_PAGE_DBLWR_FULL_FLUSH_PAGE:
      DBUG_EXECUTE_IF("crash_before_full_block_dblwr_flush", DBUG_SUICIDE(););
      break;
  }

  err = s_dblwr_file_ctx.write(from_file, from_buffer,
                               offset * ARCH_PAGE_BLK_SIZE, write_size);

  if (err != DB_SUCCESS) {
    return (err);
  }

  s_dblwr_file_ctx.flush();

  return (err);
}

dberr_t Arch_Group::init_dblwr_file_ctx(const char *path, const char *base_file,
                                        uint num_files, uint64_t file_size) {
  auto err =
      s_dblwr_file_ctx.init(path, nullptr, base_file, num_files, file_size);

  if (err != DB_SUCCESS) {
    ut_ad(s_dblwr_file_ctx.get_phy_size() == file_size);
    return (err);
  }

  err = s_dblwr_file_ctx.open(false, LSN_MAX, 0, 0);

  if (err != DB_SUCCESS) {
    return (err);
  }

  byte *buf = static_cast<byte *>(ut_zalloc(file_size, mem_key_archive));

  /* Make sure that the physical file size is the same as logical by filling
  the file with all-zeroes. Page archiver recovery expects that the physical
  file size is the same as logical file size. */
  err = s_dblwr_file_ctx.write(nullptr, buf, 0, file_size);

  if (err == DB_SUCCESS) {
    s_dblwr_file_ctx.flush();
  }

  ut_free(buf);

  return (err);
}

dberr_t Arch_Group::build_active_file_name() {
  char dir_name[MAX_ARCH_DIR_NAME_LEN];
  auto length = MAX_ARCH_DIR_NAME_LEN + 1 + MAX_ARCH_PAGE_FILE_NAME_LEN + 1;

  if (m_active_file_name != nullptr) {
    return (DB_SUCCESS);
  }

  m_active_file_name = static_cast<char *>(ut_malloc(length, mem_key_archive));

  if (m_active_file_name == nullptr) {
    return (DB_OUT_OF_MEMORY);
  }

  get_dir_name(dir_name, MAX_ARCH_DIR_NAME_LEN);

  snprintf(m_active_file_name, length, "%s%c%s", dir_name, OS_PATH_SEPARATOR,
           ARCH_PAGE_GROUP_ACTIVE_FILE_NAME);

  return (DB_SUCCESS);
}

dberr_t Arch_Group::build_durable_file_name() {
  char dir_name[MAX_ARCH_DIR_NAME_LEN];
  auto length = MAX_ARCH_DIR_NAME_LEN + 1 + MAX_ARCH_PAGE_FILE_NAME_LEN + 1;

  if (m_durable_file_name != nullptr) {
    return (DB_SUCCESS);
  }

  m_durable_file_name = static_cast<char *>(ut_malloc(length, mem_key_archive));

  if (m_durable_file_name == nullptr) {
    return (DB_OUT_OF_MEMORY);
  }

  get_dir_name(dir_name, MAX_ARCH_DIR_NAME_LEN);

  snprintf(m_durable_file_name, length, "%s%c%s", dir_name, OS_PATH_SEPARATOR,
           ARCH_PAGE_GROUP_DURABLE_FILE_NAME);

  return (DB_SUCCESS);
}


int Arch_Group::mark_active() {
  dberr_t db_err = build_active_file_name();

  if (db_err != DB_SUCCESS) {
    return (ER_OUTOFMEMORY);
  }

  os_file_create_t option;
  os_file_type_t type;

  bool success;
  bool exists;

  success = os_file_status(m_active_file_name, &exists, &type);

  if (!success) {
    return (ER_CANT_OPEN_FILE);
  }

  ut_ad(!exists);
  option = OS_FILE_CREATE_PATH;

  ut_ad(m_active_file.m_file == OS_FILE_CLOSED);

  m_active_file =
      os_file_create(innodb_arch_file_key, m_active_file_name, option,
                     OS_FILE_NORMAL, OS_CLONE_LOG_FILE, false, &success);

  int err = (success ? 0 : ER_CANT_OPEN_FILE);

  return (err);
}

int Arch_Group::mark_durable() {
  dberr_t db_err = build_durable_file_name();

  if (db_err != DB_SUCCESS) {
    return (ER_OUTOFMEMORY);
  }

  os_file_create_t option;
  os_file_type_t type;

  bool success;
  bool exists;

  success = os_file_status(m_durable_file_name, &exists, &type);

  if (exists) {
    return (0);
  }

  if (!success) {
    return (ER_CANT_OPEN_FILE);
  }

  option = OS_FILE_CREATE_PATH;

  ut_ad(m_durable_file.m_file == OS_FILE_CLOSED);

  m_durable_file =
      os_file_create(innodb_arch_file_key, m_durable_file_name, option,
                     OS_FILE_NORMAL, OS_CLONE_LOG_FILE, false, &success);

  int err = (success ? 0 : ER_CANT_OPEN_FILE);

  return (err);
}

int Arch_Group::mark_inactive() {
  os_file_type_t type;

  bool success;
  bool exists;

  dberr_t db_err;

  db_err = build_active_file_name();

  if (db_err != DB_SUCCESS) {
    return (ER_OUTOFMEMORY);
  }

  success = os_file_status(m_active_file_name, &exists, &type);

  if (!success) {
    return (ER_CANT_OPEN_FILE);
  }

  if (!exists) {
    return (0);
  }

  if (m_active_file.m_file != OS_FILE_CLOSED) {
    os_file_close(m_active_file);
    m_active_file.m_file = OS_FILE_CLOSED;
  }

  success = os_file_delete(innodb_arch_file_key, m_active_file_name);

  int err = (success ? 0 : ER_CANT_OPEN_FILE);

  return (err);
}

dberr_t Arch_Group::write_file_header(byte *from_buffer, uint length) {
  dberr_t err;

  ut_ad(!m_file_ctx.is_closed());

  /* Write to the doublewrite buffer before writing to the actual file */
  Arch_Group::write_to_doublewrite_file(nullptr, from_buffer, length,
                                        ARCH_PAGE_DBLWR_RESET_PAGE);

  DBUG_EXECUTE_IF("crash_after_reset_block_dblwr_flush", DBUG_SUICIDE(););

  err = m_file_ctx.write(nullptr, from_buffer, 0, length);

  if (err == DB_SUCCESS) {
    /* Flush the file to make sure the changes are made persistent as there
    would be no way to recover the data otherwise in case of a crash. */
    m_file_ctx.flush();
  }

  return (err);
}

dberr_t Arch_Group::open_file_during_recovery(Arch_Page_Pos write_pos,
                                              bool empty_file) {
  dberr_t err;
  uint offset;

  auto count = get_file_count();

  ut_ad(count > 0);

  if (!empty_file) {
    ut_ad(Arch_Block::get_file_index(write_pos.m_block_num) == count - 1);

    offset =
        Arch_Block::get_file_offset(write_pos.m_block_num, ARCH_DATA_BLOCK);

    err = m_file_ctx.open(false, m_begin_lsn, count - 1, offset);

  } else {
    err = m_file_ctx.open_new(m_begin_lsn, m_header_len);
  }

  return (err);
}

uint Arch_Group::purge(lsn_t purge_lsn, lsn_t &group_purged_lsn) {
  ut_ad(mutex_own(m_arch_mutex));

  if (m_begin_lsn > purge_lsn) {
    group_purged_lsn = LSN_MAX;
    return (0);
  }

  /** For a group (active or non-active) if there are any non-durable clients
  attached then we don't purge the group at all. */
  if (m_ref_count > 0) {
    group_purged_lsn = LSN_MAX;
    return (ER_PAGE_TRACKING_CANNOT_PURGE);
  }

  if (!m_is_active && m_end_lsn <= purge_lsn) {
    m_file_ctx.delete_files(m_begin_lsn);
    group_purged_lsn = m_end_lsn;
    return (0);
  }

  lsn_t purged_lsn = m_file_ctx.purge(m_begin_lsn, m_end_lsn, purge_lsn);

  group_purged_lsn = purged_lsn;

  return (0);
}
