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
#include <innodb/log_arch/Arch_Dblwr_Ctx.h>
#include <innodb/log_arch/arch_remove_dir.h>
#include <innodb/log_arch/arch_remove_file.h>
#include <innodb/log_arch/Arch_Recv_Group_Info.h>
#include <innodb/io/IORequest.h>
#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/io/os_file_read.h>

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

dberr_t Arch_Group::recovery_replace_pages_from_dblwr(
    Arch_Dblwr_Ctx *dblwr_ctx) {
  auto ARCH_UNKNOWN_BLOCK = std::numeric_limits<uint>::max();
  uint full_flush_blk_num = ARCH_UNKNOWN_BLOCK;
  auto dblwr_blocks = dblwr_ctx->get_blocks();
  size_t num_files = get_file_count();

  ut_ad(num_files > 0);

  for (uint index = 0; index < dblwr_blocks.size(); ++index) {
    auto dblwr_block = dblwr_blocks[index];

    switch (dblwr_block.m_block_type) {
      case ARCH_RESET_BLOCK:

        ut_ad(dblwr_block.m_block_num < num_files);
        /* If the block does not belong to the last file then ignore. */
        if (dblwr_block.m_block_num != num_files - 1) {
          continue;
        } else {
          break;
        }

      case ARCH_DATA_BLOCK: {
        uint file_index = Arch_Block::get_file_index(dblwr_block.m_block_num);
        ut_ad(file_index < num_files);

        /* If the block does not belong to the last file then ignore. */
        if (file_index < num_files - 1) {
          continue;
        }

        if (dblwr_block.m_flush_type == ARCH_FLUSH_NORMAL) {
          full_flush_blk_num = dblwr_block.m_block_num;
        } else {
          /* It's possible that the partial flush block might have been fully
          flushed, in which case we need to skip this block. */
          if (full_flush_blk_num != ARCH_UNKNOWN_BLOCK &&
              full_flush_blk_num >= dblwr_block.m_block_num) {
            continue;
          }
        }
      } break;

      default:
        ut_ad(false);
    }

    uint64_t offset = Arch_Block::get_file_offset(dblwr_block.m_block_num,
                                                  dblwr_block.m_block_type);

    ut_ad(m_file_ctx.is_closed());

    dberr_t err;

    err = m_file_ctx.open(false, m_begin_lsn, num_files - 1, 0);

    if (err != DB_SUCCESS) {
      return (err);
    }

    err = m_file_ctx.write(nullptr, dblwr_block.m_block, offset,
                           ARCH_PAGE_BLK_SIZE);

    if (err != DB_SUCCESS) {
      return (err);
    }

    m_file_ctx.close();
  }

  return (DB_SUCCESS);
}

dberr_t Arch_Group::recovery_cleanup_if_required(uint &num_files,
                                                 uint start_index, bool durable,
                                                 bool &empty_file) {
  dberr_t err;

  ut_ad(!durable || num_files > 0);
  ut_ad(m_file_ctx.is_closed());

  uint index = start_index + num_files - 1;

  /* Open the last file in the group. */
  err = m_file_ctx.open(true, m_begin_lsn, index, 0);

  if (err != DB_SUCCESS) {
    return (err);
  }

  if (m_file_ctx.get_phy_size() != 0 && durable) {
    m_file_ctx.close();
    return (DB_SUCCESS);
  }

  empty_file = true;

  /* No blocks have been flushed into the file so delete the file. */

  char file_path[MAX_ARCH_PAGE_FILE_NAME_LEN];
  char dir_path[MAX_ARCH_DIR_NAME_LEN];

  m_file_ctx.build_name(index, m_begin_lsn, file_path,
                        MAX_ARCH_PAGE_FILE_NAME_LEN);

  auto found = std::string(file_path).find(ARCH_PAGE_FILE);
  ut_ad(found != std::string::npos);
  auto file_name = std::string(file_path).substr(found);

  m_file_ctx.build_dir_name(m_begin_lsn, dir_path, MAX_ARCH_DIR_NAME_LEN);

  m_file_ctx.close();

  arch_remove_file(dir_path, file_name.c_str());

  --num_files;

  /* If there are no archive files in the group we might as well
  purge it. */
  if (num_files == 0 || !durable) {
    m_is_active = false;

    found = std::string(dir_path).find(ARCH_PAGE_DIR);
    ut_ad(found != std::string::npos);

    auto path = std::string(dir_path).substr(0, found - 1);
    auto dir_name = std::string(dir_path).substr(found);

    num_files = 0;
    arch_remove_dir(path.c_str(), dir_name.c_str());
  }

  /* Need to reinitialize the file context as num_files has changed. */
  err = m_file_ctx.init(
      ARCH_DIR, ARCH_PAGE_DIR, ARCH_PAGE_FILE, num_files,
      static_cast<uint64_t>(ARCH_PAGE_BLK_SIZE) * ARCH_PAGE_FILE_CAPACITY);

  return (err);
}

dberr_t Arch_Group::recover(Arch_Recv_Group_Info *group_info,
                            bool &new_empty_file, Arch_Dblwr_Ctx *dblwr_ctx,
                            Arch_Page_Pos &write_pos,
                            Arch_Page_Pos &reset_pos) {
  dberr_t err;

  err = init_file_ctx(
      ARCH_DIR, ARCH_PAGE_DIR, ARCH_PAGE_FILE, group_info->m_num_files,
      static_cast<uint64_t>(ARCH_PAGE_BLK_SIZE) * ARCH_PAGE_FILE_CAPACITY);

  if (err != DB_SUCCESS) {
    return (err);
  }

  if (group_info->m_active) {
    /* Since the group was active at the time of crash it's possible that the
    doublewrite buffer might have the latest data in case of a crash. */

    err = recovery_replace_pages_from_dblwr(dblwr_ctx);

    if (err != DB_SUCCESS) {
      return (err);
    }
  }

  err = recovery_cleanup_if_required(group_info->m_num_files,
                                     group_info->m_file_start_index,
                                     group_info->m_durable, new_empty_file);

  if (err != DB_SUCCESS) {
    return (err);
  }

  if (group_info->m_num_files == 0) {
    return (err);
  }

  err = recovery_parse(write_pos, reset_pos, group_info->m_file_start_index);

  if (err != DB_SUCCESS) {
    return (err);
  }

  if (!group_info->m_active) {
    /* Group was inactive at the time of shutdown/crash, so
    we just add the group to the group list that the
    archiver maintains. */

    attach_during_recovery();
    m_stop_pos = write_pos;

    auto end_lsn = m_file_ctx.get_last_stop_point();
    ut_ad(end_lsn != LSN_MAX);

    disable(end_lsn);
  } else {
    err = open_file_during_recovery(write_pos, new_empty_file);
  }

  ut_d(m_file_ctx.recovery_reset_print(group_info->m_file_start_index));

  return (err);
}

dberr_t Arch_Group::recovery_parse(Arch_Page_Pos &write_pos,
                                   Arch_Page_Pos &reset_pos,
                                   size_t start_index) {
  size_t num_files = get_file_count();
  dberr_t err = DB_SUCCESS;

  if (num_files == 0) {
    DBUG_PRINT("page_archiver", ("No group information available"));
    return (DB_SUCCESS);
  }

  ut_ad(m_file_ctx.is_closed());

  uint file_count = start_index + num_files;

  for (auto file_index = start_index; file_index < file_count; ++file_index) {
    if (file_index == start_index) {
      err = m_file_ctx.open(true, m_begin_lsn, start_index, 0);
    } else {
      err = m_file_ctx.open_next(m_begin_lsn, 0);
    }

    if (err != DB_SUCCESS) {
      break;
    }

    err = m_file_ctx.fetch_reset_points(file_index, reset_pos);

    if (err != DB_SUCCESS) {
      break;
    }

    bool last_file = (file_index + 1 == file_count);
    err = m_file_ctx.fetch_stop_points(last_file, write_pos);

    if (err != DB_SUCCESS) {
      break;
    }

    m_file_ctx.close();
  }

  m_file_ctx.close();

  return (err);
}

dberr_t Arch_Group::recovery_read_latest_blocks(byte *buf, uint64_t offset,
                                                Arch_Blk_Type type) {
  dberr_t err = DB_SUCCESS;

  ut_ad(!m_file_ctx.is_closed());

  switch (type) {
    case ARCH_RESET_BLOCK: {
      ut_d(uint64_t file_size = m_file_ctx.get_phy_size());
      ut_ad((file_size > ARCH_PAGE_FILE_NUM_RESET_PAGE * ARCH_PAGE_BLK_SIZE) &&
            (file_size % ARCH_PAGE_BLK_SIZE == 0));

      err = m_file_ctx.read(buf, 0, ARCH_PAGE_BLK_SIZE);
    } break;

    case ARCH_DATA_BLOCK:
      err = m_file_ctx.read(buf, offset, ARCH_PAGE_BLK_SIZE);
      break;
  }

  return (err);
}

#ifdef UNIV_DEBUG
void Arch_Group::adjust_end_lsn(lsn_t &stop_lsn, uint32_t &blk_len) {
  stop_lsn = ut_uint64_align_down(get_begin_lsn(), OS_FILE_LOG_BLOCK_SIZE);

  stop_lsn += get_file_size() - LOG_FILE_HDR_SIZE;
  blk_len = 0;

  /* Increase Stop LSN 64 bytes ahead of file end not exceeding
  redo block size. */
  DBUG_EXECUTE_IF("clone_arch_log_extra_bytes",
                  blk_len = OS_FILE_LOG_BLOCK_SIZE;
                  stop_lsn += 64;);
}
#endif /* UNIV_DEBUG */

int Arch_Group::read_from_file(Arch_Page_Pos *read_pos, uint read_len,
                               byte *read_buff) {
  char errbuf[MYSYS_STRERROR_SIZE];
  char file_name[MAX_ARCH_PAGE_FILE_NAME_LEN];

  /* Build file name */
  auto file_index =
      static_cast<uint>(Arch_Block::get_file_index(read_pos->m_block_num));

  get_file_name(file_index, file_name, MAX_ARCH_PAGE_FILE_NAME_LEN);

  /* Find offset to read from. */
  os_offset_t offset =
      Arch_Block::get_file_offset(read_pos->m_block_num, ARCH_DATA_BLOCK);
  offset += read_pos->m_offset;

  bool success;

  /* Open file in read only mode. */
  pfs_os_file_t file =
      os_file_create(innodb_arch_file_key, file_name, OS_FILE_OPEN,
                     OS_FILE_NORMAL, OS_CLONE_LOG_FILE, true, &success);

  if (!success) {
    my_error(ER_CANT_OPEN_FILE, MYF(0), file_name, errno,
             my_strerror(errbuf, sizeof(errbuf), errno));

    return (ER_CANT_OPEN_FILE);
  }

  /* Read from file to the user buffer. */
  IORequest request(IORequest::READ);

  request.disable_compression();
  request.clear_encrypted();

  auto db_err = os_file_read(request, file, read_buff, offset, read_len);

  os_file_close(file);

  if (db_err != DB_SUCCESS) {
    my_error(ER_ERROR_ON_READ, MYF(0), file_name, errno,
             my_strerror(errbuf, sizeof(errbuf), errno));
    return (ER_ERROR_ON_READ);
  }

  return (0);
}

int Arch_Group::read_data(Arch_Page_Pos cur_pos, byte *buff, uint buff_len) {
  int err = 0;

  /* Attempt to read from in memory buffer. */
  auto success = arch_page_sys->get_pages(this, &cur_pos, buff_len, buff);

  if (!success) {
    /* The buffer is overwritten. Read from file. */
    err = read_from_file(&cur_pos, buff_len, buff);
  }

  return (err);
}
