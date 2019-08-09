#include <innodb/log_arch/Arch_File_Ctx.h>

#include <innodb/allocator/ut_malloc.h>
#include <innodb/formatting/formatting.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/create_purpose.h>
#include <innodb/io/create_type.h>
#include <innodb/io/os_file_copy.h>
#include <innodb/io/os_file_create.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/os_file_read.h>
#include <innodb/io/os_file_scan_directory.h>
#include <innodb/io/os_file_seek.h>
#include <innodb/io/os_file_status.h>
#include <innodb/io/os_file_type_t.h>
#include <innodb/io/os_file_write.h>
#include <innodb/io/pfs.h>
#include <innodb/log_arch/arch_remove_file.h>
#include <innodb/log_arch/flags.h>
#include <innodb/log_arch/Arch_Block.h>
#include <innodb/log_arch/arch_page_sys.h>
#include <innodb/logger/info.h>

#include <algorithm>

void Arch_File_Ctx::delete_files(lsn_t begin_lsn) {
  bool exists = false;
  char dir_name[MAX_ARCH_DIR_NAME_LEN];

  os_file_type_t type;

  build_dir_name(begin_lsn, dir_name, MAX_ARCH_DIR_NAME_LEN);
  os_file_status(dir_name, &exists, &type);

  if (exists) {
    ut_ad(type == OS_FILE_TYPE_DIR);
    os_file_scan_directory(dir_name, arch_remove_file, true);
  }
}


dberr_t Arch_File_Ctx::init(const char *path, const char *base_dir,
                            const char *base_file, uint num_files,
                            uint64_t file_size) {
  m_base_len = static_cast<uint>(strlen(path));

  m_name_len =
      m_base_len + static_cast<uint>(strlen(base_file)) + MAX_LSN_DECIMAL_DIGIT;

  if (base_dir != nullptr) {
    m_name_len += static_cast<uint>(strlen(base_dir));
    m_name_len += MAX_LSN_DECIMAL_DIGIT;
  }

  /* Add some extra buffer. */
  m_name_len += MAX_LSN_DECIMAL_DIGIT;

  /* In case of reinitialise. */
  if (m_name_buf != nullptr) {
    ut_free(m_name_buf);
    m_name_buf = nullptr;
  }

  m_name_buf = static_cast<char *>(ut_malloc(m_name_len, mem_key_archive));

  if (m_name_buf == nullptr) {
    return (DB_OUT_OF_MEMORY);
  }

  m_path_name = path;
  m_dir_name = base_dir;
  m_file_name = base_file;

  strcpy(m_name_buf, path);

  if (m_name_buf[m_base_len - 1] != OS_PATH_SEPARATOR) {
    m_name_buf[m_base_len] = OS_PATH_SEPARATOR;
    ++m_base_len;
    m_name_buf[m_base_len] = '\0';
  }

  m_file.m_file = OS_FILE_CLOSED;

  m_index = 0;
  m_count = num_files;

  m_offset = 0;
  m_size = file_size;

  m_reset.clear();
  m_stop_points.clear();

  return (DB_SUCCESS);
}

dberr_t Arch_File_Ctx::open(bool read_only, lsn_t start_lsn, uint file_index,
                            uint64_t file_offset) {
  os_file_create_t option;
  os_file_type_t type;

  bool success;
  bool exists;

  /* Close current file, if open. */
  close();

  m_index = file_index;
  m_offset = file_offset;

  ut_ad(m_offset <= m_size);

  build_name(m_index, start_lsn, nullptr, 0);

  success = os_file_status(m_name_buf, &exists, &type);

  if (!success) {
    return (DB_CANNOT_OPEN_FILE);
  }

  if (read_only) {
    if (!exists) {
      return (DB_CANNOT_OPEN_FILE);
    }

    option = OS_FILE_OPEN;
  } else {
    option = exists ? OS_FILE_OPEN : OS_FILE_CREATE_PATH;
  }

  m_file =
      os_file_create(innodb_arch_file_key, m_name_buf, option, OS_FILE_NORMAL,
                     OS_CLONE_LOG_FILE, read_only, &success);

  if (!success) {
    return (DB_CANNOT_OPEN_FILE);
  }

  success = os_file_seek(m_name_buf, m_file.m_file, file_offset);

  return (success ? DB_SUCCESS : DB_IO_ERROR);
}

dberr_t Arch_File_Ctx::open_new(lsn_t start_lsn, uint64_t file_offset) {
  dberr_t error;

  /* Create and open next file. */
  error = open(false, start_lsn, m_count, file_offset);

  if (error != DB_SUCCESS) {
    return (error);
  }

  /* Increase file count. */
  ++m_count;
  return (DB_SUCCESS);
}

dberr_t Arch_File_Ctx::open_next(lsn_t start_lsn, uint64_t file_offset) {
  dberr_t error;

  /* Get next file index. */
  ++m_index;
  if (m_index == m_count) {
    m_index = 0;
  }

  /* Open next file. */
  error = open(true, start_lsn, m_index, file_offset);

  return (error);
}

dberr_t Arch_File_Ctx::read(byte *to_buffer, uint offset, uint size) {
  ut_ad(offset + size <= m_size);
  ut_ad(!is_closed());

  IORequest request(IORequest::READ);
  request.disable_compression();
  request.clear_encrypted();

  auto err = os_file_read(request, m_file, to_buffer, offset, size);

  return (err);
}

dberr_t Arch_File_Ctx::write(Arch_File_Ctx *from_file, byte *from_buffer,
                             uint size) {
  dberr_t err;

  if (from_buffer == nullptr) {
    /* write from File */
    err = os_file_copy(from_file->m_file, from_file->m_offset, m_file, m_offset,
                       size);

    if (err == DB_SUCCESS) {
      from_file->m_offset += size;
      ut_ad(from_file->m_offset <= from_file->m_size);
    }

  } else {
    /* write from buffer */
    IORequest request(IORequest::WRITE);
    request.disable_compression();
    request.clear_encrypted();

    err = os_file_write(request, "Track file", m_file, from_buffer, m_offset,
                        size);
  }

  if (err != DB_SUCCESS) {
    return (err);
  }

  m_offset += size;
  ut_ad(m_offset <= m_size);

  return (DB_SUCCESS);
}

void Arch_File_Ctx::build_name(uint idx, lsn_t dir_lsn, char *buffer,
                               uint length) {
  char *buf_ptr;
  uint buf_len;

  /* If user has passed NULL, use pre-allocated buffer. */
  if (buffer == nullptr) {
    buf_ptr = m_name_buf;
    buf_len = m_name_len;
  } else {
    buf_ptr = buffer;
    buf_len = length;

    strncpy(buf_ptr, m_name_buf, buf_len);
  }

  buf_ptr += m_base_len;
  buf_len -= m_base_len;

  if (m_dir_name == nullptr) {
    snprintf(buf_ptr, buf_len, "%s%u", m_file_name, idx);
  } else {
    snprintf(buf_ptr, buf_len, "%s" UINT64PF "%c%s%u", m_dir_name, dir_lsn,
             OS_PATH_SEPARATOR, m_file_name, idx);
  }
}

void Arch_File_Ctx::build_dir_name(lsn_t dir_lsn, char *buffer, uint length) {
  ut_ad(buffer != nullptr);

  if (m_dir_name != nullptr) {
    snprintf(buffer, length, "%s%c%s" UINT64PF, m_path_name, OS_PATH_SEPARATOR,
             m_dir_name, dir_lsn);
  } else {
    snprintf(buffer, length, "%s", m_path_name);
  }
}

void Arch_File_Ctx::update_stop_point(uint file_index, lsn_t stop_lsn) {
  auto last_point_index = m_stop_points.size() - 1;

  if (!m_stop_points.size() || last_point_index != file_index) {
    m_stop_points.push_back(stop_lsn);
  } else {
    m_stop_points[last_point_index] = stop_lsn;
  }
}

void Arch_File_Ctx::save_reset_point_in_mem(lsn_t lsn, Arch_Page_Pos pos) {
  uint current_file_index = Arch_Block::get_file_index(pos.m_block_num);

  Arch_Point reset_point;
  reset_point.lsn = lsn;
  reset_point.pos = pos;

  Arch_Reset_File reset_file;

  if (m_reset.size()) {
    reset_file = m_reset.back();

    if (reset_file.m_file_index == current_file_index) {
      reset_file.m_start_point.push_back(reset_point);
      m_reset[m_reset.size() - 1] = reset_file;
      return;
    }
  }

  /* Reset info maintained in a new file. */
  reset_file.init();
  reset_file.m_file_index = current_file_index;
  reset_file.m_lsn = lsn;
  reset_file.m_start_point.push_back(reset_point);
  m_reset.push_back(reset_file);
}

bool Arch_File_Ctx::find_reset_point(lsn_t check_lsn, Arch_Point &reset_point) {
  if (!m_reset.size()) {
    return (false);
  }

  Arch_Reset_File file_reset_compare;
  file_reset_compare.m_lsn = check_lsn;

  /* Finds the file which has the element that is >= to check_lsn */
  auto reset_it = std::lower_bound(
      m_reset.begin(), m_reset.end(), file_reset_compare,
      [](const Arch_Reset_File &lhs, const Arch_Reset_File &rhs) {
        return (lhs.m_lsn < rhs.m_lsn);
      });

  if (reset_it != m_reset.end() && reset_it->m_lsn == check_lsn) {
    reset_point = reset_it->m_start_point.front();
    return (true);
  }

  if (reset_it == m_reset.begin()) {
    return (false);
  }

  /* The element that is less than check_lsn, which we're interested in,
  will be in the previous position. */
  --reset_it;
  ut_ad(reset_it->m_lsn < check_lsn);

  auto reset_file = *reset_it;
  auto reset_start_point = reset_file.m_start_point;

  Arch_Point reset_point_compare;
  reset_point_compare.lsn = check_lsn;

  /* Find the first start point whose lsn is >= to check_lsn. */
  auto reset_point_it = std::lower_bound(
      reset_start_point.begin(), reset_start_point.end(), reset_point_compare,
      [](const Arch_Point &lhs, const Arch_Point &rhs) {
        return (lhs.lsn < rhs.lsn);
      });

  if (reset_point_it == reset_start_point.end() ||
      reset_point_it->lsn != check_lsn) {
    ut_ad(reset_point_it != reset_start_point.begin());
    --reset_point_it;
  }

  reset_point = *reset_point_it;

  return (true);
}

dberr_t Arch_File_Ctx::write(Arch_File_Ctx *from_file, byte *from_buffer,
                             uint offset, uint size) {
  dberr_t err;

  ut_ad(offset + size <= m_size);
  ut_ad(!is_closed());

  if (from_buffer == nullptr) {
    ut_ad(offset + size <= from_file->get_size());
    ut_ad(!from_file->is_closed());

    err = os_file_copy(from_file->m_file, offset, m_file, offset, size);
  } else {
    IORequest request(IORequest::WRITE);
    request.disable_compression();
    request.clear_encrypted();

    err = os_file_write(request, "Page Track File", m_file, from_buffer, offset,
                        size);
  }

  return (err);
}

bool Arch_File_Ctx::find_stop_point(Arch_Group *group, lsn_t check_lsn,
                                    Arch_Point &stop_point,
                                    Arch_Page_Pos last_pos) {
  stop_point.lsn = LSN_MAX;
  stop_point.pos.init();

  arch_page_sys->arch_oper_mutex_enter();

  if (!m_stop_points.size()) {
    arch_page_sys->arch_oper_mutex_exit();
    return (false);
  }

  ut_ad(m_stop_points.back() <= arch_page_sys->get_latest_stop_lsn());

  /* 1. Find the file where the block we need to stop at is present */

  uint file_index = 0;

  for (uint i = 0; i < m_stop_points.size(); ++i) {
    file_index = i;

    if (m_stop_points[i] >= check_lsn) {
      break;
    }
  }

  ut_ad((m_stop_points[file_index] >= check_lsn &&
         (file_index == 0 || m_stop_points[file_index - 1] < check_lsn)));

  arch_page_sys->arch_oper_mutex_exit();

  /* 2. Find the block in the file where to stop. */

  byte header_buf[ARCH_PAGE_BLK_HEADER_LENGTH];

  Arch_Page_Pos left_pos;
  left_pos.m_block_num = ARCH_PAGE_FILE_DATA_CAPACITY * file_index;

  Arch_Page_Pos right_pos;

  if (file_index < m_stop_points.size() - 1) {
    right_pos.m_block_num =
        left_pos.m_block_num + ARCH_PAGE_FILE_DATA_CAPACITY - 1;
  } else {
    right_pos.m_block_num = last_pos.m_block_num;
  }

  lsn_t block_stop_lsn;
  int err;

  while (left_pos.m_block_num <= right_pos.m_block_num) {
    Arch_Page_Pos middle_pos;
    middle_pos.init();
    middle_pos.m_offset = 0;

    middle_pos.m_block_num = left_pos.m_block_num +
                             (right_pos.m_block_num - left_pos.m_block_num) / 2;

    /* Read the block header for data length and stop lsn info. */
    err = group->read_data(middle_pos, header_buf, ARCH_PAGE_BLK_HEADER_LENGTH);

    if (err != 0) {
      return (false);
    }

    block_stop_lsn = Arch_Block::get_stop_lsn(header_buf);
    auto data_len = Arch_Block::get_data_len(header_buf);

    middle_pos.m_offset = data_len + ARCH_PAGE_BLK_HEADER_LENGTH;

    if (block_stop_lsn >= check_lsn) {
      stop_point.lsn = block_stop_lsn;
      stop_point.pos = middle_pos;
    }

    if (left_pos.m_block_num == right_pos.m_block_num ||
        block_stop_lsn == check_lsn) {
      break;
    }

    if (block_stop_lsn > check_lsn) {
      right_pos.m_block_num = middle_pos.m_block_num - 1;
    } else {
      left_pos.m_block_num = middle_pos.m_block_num + 1;
    }
  }

  ut_ad(stop_point.lsn != LSN_MAX);

  return (true);
}


#ifdef UNIV_DEBUG

bool Arch_File_Ctx::validate_stop_point_in_file(Arch_Group *group,
                                                pfs_os_file_t file,
                                                uint file_index) {
  lsn_t stop_lsn = LSN_MAX;
  bool last_file = file_index + 1 == m_count;

  if (last_file && group->is_active() && group->get_end_lsn() == LSN_MAX) {
    /* Just return true if this is the case as the block might not have been
    flushed to disk yet */
    return (true);
  }

  if (file_index >= m_stop_points.size()) {
    ut_ad(false);
    return (false);
  }

  /* Read from file to the user buffer. */
  IORequest request(IORequest::READ);
  request.disable_compression();
  request.clear_encrypted();

  uint64_t offset;

  if (!last_file) {
    offset = ARCH_PAGE_FILE_DATA_CAPACITY * ARCH_PAGE_BLK_SIZE;
  } else {
    offset = Arch_Block::get_file_offset(group->get_stop_pos().m_block_num,
                                         ARCH_DATA_BLOCK);
  }

  byte buf[ARCH_PAGE_BLK_SIZE];

  /* Read the entire reset block. */
  dberr_t err = os_file_read(request, file, buf, offset, ARCH_PAGE_BLK_SIZE);

  if (err != DB_SUCCESS) {
    return (false);
  }

  stop_lsn = Arch_Block::get_stop_lsn(buf);

  if (stop_lsn != m_stop_points[file_index]) {
    ut_ad(false);
    return (false);
  }

  DBUG_PRINT("page_archiver", ("File stop point: %" PRIu64 "", stop_lsn));

  return (true);
}

bool Arch_File_Ctx::validate_reset_block_in_file(pfs_os_file_t file,
                                                 uint file_index,
                                                 uint &reset_count) {
  /* Read from file to the user buffer. */
  IORequest request(IORequest::READ);
  request.disable_compression();
  request.clear_encrypted();

  byte buf[ARCH_PAGE_BLK_SIZE];

  /* Read the entire reset block. */
  dberr_t err = os_file_read(request, file, buf, 0, ARCH_PAGE_BLK_SIZE);

  if (err != DB_SUCCESS) {
    return (false);
  }

  auto data_length = Arch_Block::get_data_len(buf);

  if (data_length == 0) {
    /* No reset, move to the next file. */
    return (true);
  }

  ut_ad(data_length >= ARCH_PAGE_FILE_HEADER_RESET_LSN_SIZE +
                           ARCH_PAGE_FILE_HEADER_RESET_POS_SIZE);

  Arch_Reset_File reset_file;

  if (!m_reset.size() || reset_count >= m_reset.size()) {
    ut_ad(false);
    return (false);
  }

  reset_file = m_reset.at(reset_count);

  if (reset_file.m_file_index != file_index) {
    ut_ad(false);
    return (false);
  }

  byte *block_data = buf + ARCH_PAGE_BLK_HEADER_LENGTH;

  lsn_t file_reset_lsn = mach_read_from_8(block_data);
  uint length = ARCH_PAGE_FILE_HEADER_RESET_LSN_SIZE;

  if (reset_file.m_lsn != file_reset_lsn) {
    ut_ad(false);
    return (false);
  }

  DBUG_PRINT("page_archiver", ("File lsn : %" PRIu64 "", file_reset_lsn));

  uint index = 0;
  Arch_Point start_point;

  while (length < data_length) {
    if (index >= reset_file.m_start_point.size()) {
      ut_ad(false);
      return (false);
    }

    start_point = reset_file.m_start_point.at(index);

    uint64_t block_num = mach_read_from_2(block_data + length);
    length += ARCH_PAGE_FILE_HEADER_RESET_BLOCK_NUM_SIZE;

    uint64_t block_offset = mach_read_from_2(block_data + length);
    length += ARCH_PAGE_FILE_HEADER_RESET_BLOCK_OFFSET_SIZE;

    if (block_num != start_point.pos.m_block_num ||
        block_offset != start_point.pos.m_offset) {
      ut_ad(false);
      return (false);
    }

    DBUG_PRINT("page_archiver",
               ("Reset point %u : %" PRIu64 ", %" PRIu64 ", %" PRIu64 "", index,
                start_point.lsn, block_num, block_offset));

    ++index;
  }

  ut_ad(length == data_length);

  if (reset_file.m_start_point.size() != index) {
    ut_ad(false);
    return (false);
  }

  ++reset_count;

  return (true);
}

bool Arch_Group::validate_info_in_files() {
  uint reset_count = 0;
  uint file_count = m_file_ctx.get_count();
  bool success = true;

  DBUG_PRINT("page_archiver", ("RESET PAGE"));

  for (uint file_index = 0; file_index < file_count; ++file_index) {
    bool last_file = file_index + 1 == file_count;

    if (last_file && m_file_ctx.get_phy_size() == 0) {
      success = false;
      break;
    }

    success = m_file_ctx.validate(this, file_index, m_begin_lsn, reset_count);

    if (!success) {
      break;
    }
  }

  DBUG_PRINT("page_archiver", ("\n"));

  return (success);
}

bool Arch_File_Ctx::validate(Arch_Group *group, uint file_index,
                             lsn_t start_lsn, uint &reset_count) {
  char file_name[MAX_ARCH_PAGE_FILE_NAME_LEN];

  build_name(file_index, start_lsn, file_name, MAX_ARCH_PAGE_FILE_NAME_LEN);

  os_file_type_t type;
  bool exists = false;
  bool success = os_file_status(file_name, &exists, &type);

  if (!success || !exists) {
    /* Could be the case if files are purged. */
    return (true);
  }

  pfs_os_file_t file;

  file = os_file_create(innodb_arch_file_key, file_name, OS_FILE_OPEN,
                        OS_FILE_NORMAL, OS_CLONE_LOG_FILE, true, &success);

  if (!success) {
    return (false);
  }

  DBUG_PRINT("page_archiver", ("File : %u", file_index));

  success = validate_reset_block_in_file(file, file_index, reset_count);

  if (!success) {
    ut_ad(false);
    if (file.m_file != OS_FILE_CLOSED) {
      os_file_close(file);
    }
    return (false);
  }

  success = validate_stop_point_in_file(group, file, file_index);

  if (file.m_file != OS_FILE_CLOSED) {
    os_file_close(file);
  }

  if (!success ||
      (file_index + 1 == m_count && reset_count != m_reset.size())) {
    ut_ad(false);
    return (false);
  }

  return (true);
}

#endif /* UNIV_DEBUG */




lsn_t Arch_File_Ctx::purge(lsn_t begin_lsn, lsn_t end_lsn, lsn_t purge_lsn) {
  Arch_Point reset_point;

  /* Find reset lsn which is <= purge_lsn. */
  auto success = find_reset_point(purge_lsn, reset_point);

  if (!success || reset_point.lsn == begin_lsn) {
    ib::info() << "Could not find appropriate reset points.";
    return (LSN_MAX);
  }

  ut_ad(begin_lsn < reset_point.lsn && reset_point.lsn <= end_lsn);

  Arch_Reset_File file_reset_compare;
  file_reset_compare.m_lsn = reset_point.lsn;

  /* Finds the file which has the element that is >= to reset_point.lsn. */
  auto reset_file_it = std::lower_bound(
      m_reset.begin(), m_reset.end(), file_reset_compare,
      [](const Arch_Reset_File &lhs, const Arch_Reset_File &rhs) {
        return (lhs.m_lsn < rhs.m_lsn);
      });

  /* The element that is less than check_lsn, which we're interested in,
  will be in the previous position. */
  if (reset_file_it != m_reset.begin() &&
      (reset_file_it == m_reset.end() || reset_file_it->m_lsn != purge_lsn)) {
    --reset_file_it;
  }

  if (reset_file_it == m_reset.begin()) {
    return (LSN_MAX);
  }

  lsn_t purged_lsn = reset_file_it->m_lsn;

  for (auto it = m_reset.begin(); it != reset_file_it;) {
    bool success = delete_file(it->m_file_index, begin_lsn);

    if (success) {
      /** Removes the deleted file from reset info, thereby incrementing the
       * iterator. */
      it = m_reset.erase(it);
    } else {
      ut_ad(0);
      purged_lsn = it->m_lsn;
      reset_file_it = it;
      break;
    }
  }

  /** Only files which have a reset would be purged in the above loop. We want
  to purge all the files preceding reset_file_it regardless of whether it has
  a reset or not. */
  for (uint file_index = 0; file_index < reset_file_it->m_file_index;
       ++file_index) {
    delete_file(file_index, begin_lsn);
  }

  return (purged_lsn);
}

