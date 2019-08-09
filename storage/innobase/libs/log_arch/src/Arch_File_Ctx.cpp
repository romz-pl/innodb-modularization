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
