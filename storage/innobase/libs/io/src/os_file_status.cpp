#include <innodb/io/os_file_status.h>

#include <innodb/io/os_file_status_posix.h>

/** Check the existence and type of the given file.
@param[in]	path		path name of file
@param[out]	exists		true if the file exists
@param[out]	type		Type of the file, if it exists
@return true if call succeeded */
bool os_file_status(const char *path, bool *exists, os_file_type_t *type) {
#ifdef _WIN32
  return (os_file_status_win32(path, exists, type));
#else
  return (os_file_status_posix(path, exists, type));
#endif /* _WIN32 */
}
