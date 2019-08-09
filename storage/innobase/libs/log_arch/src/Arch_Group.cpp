#include <innodb/log_arch/Arch_Group.h>

#include <innodb/io/os_file_type_t.h>
#include <innodb/io/os_file_status.h>
#include <innodb/io/pfs.h>
#include <innodb/io/os_file_delete.h>

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

