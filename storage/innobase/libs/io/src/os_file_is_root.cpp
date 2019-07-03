#include <innodb/io/os_file_is_root.h>

/** Check if the path refers to the root of a drive using a pointer
to the last directory separator that the caller has fixed.
@param[in]	path		path name
@param[in]	last_slash	last directory separator in the path
@return true if this path is a drive root, false if not */
bool os_file_is_root(const char *path, const char *last_slash) {
  return (
#ifdef _WIN32
      (last_slash == path + 2 && path[1] == ':') ||
#endif /* _WIN32 */
      last_slash == path);
}

