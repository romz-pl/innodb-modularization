#include <innodb/io/os_file_seek.h>

#include <innodb/io/os_file_handle_error_no_exit.h>

#include <sys/types.h>
#include <unistd.h>

/** Set read/write position of a file handle to specific offset.
@param[in]	pathname	file path
@param[in]	file		file handle
@param[in]	offset		read/write offset
@return true if success */
bool os_file_seek(const char *pathname, os_file_t file, os_offset_t offset) {
  bool success = true;

#ifdef _WIN32
  LARGE_INTEGER length;

  length.QuadPart = offset;

  success = SetFilePointerEx(file, length, NULL, FILE_BEGIN);

#else  /* _WIN32 */
  off_t ret;

  ret = lseek(file, offset, SEEK_SET);

  if (ret == -1) {
    success = false;
  }
#endif /* _WIN32 */

  if (!success) {
    os_file_handle_error_no_exit(pathname, "os_file_set", false);
  }

  return (success);
}
