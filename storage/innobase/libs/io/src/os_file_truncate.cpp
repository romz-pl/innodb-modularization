#include <innodb/io/os_file_truncate.h>

#include <innodb/io/os_file_truncate_posix.h>
#include <innodb/io/os_file_get_size.h>

/** Truncates a file to a specified size in bytes.
Do nothing if the size to preserve is greater or equal to the current
size of the file.
@param[in]	pathname	file path
@param[in]	file		file to be truncated
@param[in]	size		size to preserve in bytes
@return true if success */
bool os_file_truncate(const char *pathname, pfs_os_file_t file,
                      os_offset_t size) {
  /* Do nothing if the size preserved is larger than or equal to the
  current size of file */
  os_offset_t size_bytes = os_file_get_size(file);

  if (size >= size_bytes) {
    return (true);
  }

#ifdef _WIN32
  return (os_file_truncate_win32(pathname, file, size));
#else  /* _WIN32 */
  return (os_file_truncate_posix(pathname, file, size));
#endif /* _WIN32 */
}

