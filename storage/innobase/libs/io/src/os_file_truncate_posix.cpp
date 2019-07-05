#include <innodb/io/os_file_truncate_posix.h>

#include <innodb/io/os_file_handle_error_no_exit.h>
#include <innodb/logger/warn.h>

#include <unistd.h>
#include <sys/types.h>

/** Truncates a file to a specified size in bytes.
Do nothing if the size to preserve is greater or equal to the current
size of the file.
@param[in]	pathname	file path
@param[in]	file		file to be truncated
@param[in]	size		size to preserve in bytes
@return true if success */
bool os_file_truncate_posix(const char *pathname, pfs_os_file_t file,
                                   os_offset_t size) {
  int res = ftruncate(file.m_file, size);
  if (res == -1) {
    bool retry;

    retry = os_file_handle_error_no_exit(pathname, "truncate", false);

    if (retry) {
      ib::warn(ER_IB_MSG_783) << "Truncate failed for '" << pathname << "'";
    }
  }

  return (res == 0);
}
