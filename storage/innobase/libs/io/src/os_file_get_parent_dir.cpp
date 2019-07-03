#include <innodb/io/os_file_get_parent_dir.h>

#include <innodb/io/os_file_is_root.h>
#include <innodb/allocator/mem_strdupl.h>

#include <string.h>

/** Return the parent directory component of a null-terminated path.
Return a new buffer containing the string up to, but not including,
the final component of the path.
The path returned will not contain a trailing separator.
Do not return a root path, return NULL instead.
The final component trimmed off may be a filename or a directory name.
If the final component is the only component of the path, return NULL.
It is the caller's responsibility to free the returned string after it
is no longer needed.
@param[in]	path		Path name
@return own: parent directory of the path */
char *os_file_get_parent_dir(const char *path) {
  bool has_trailing_slash = false;

  /* Find the offset of the last slash */
  const char *last_slash = strrchr(path, OS_PATH_SEPARATOR);

  if (!last_slash) {
    /* No slash in the path, return NULL */
    return (NULL);
  }

  /* Ok, there is a slash. Is there anything after it? */
  if (static_cast<size_t>(last_slash - path + 1) == strlen(path)) {
    has_trailing_slash = true;
  }

  /* Reduce repetative slashes. */
  while (last_slash > path && last_slash[-1] == OS_PATH_SEPARATOR) {
    last_slash--;
  }

  /* Check for the root of a drive. */
  if (os_file_is_root(path, last_slash)) {
    return (NULL);
  }

  /* If a trailing slash prevented the first strrchr() from trimming
  the last component of the path, trim that component now. */
  if (has_trailing_slash) {
    /* Back up to the previous slash. */
    last_slash--;
    while (last_slash > path && last_slash[0] != OS_PATH_SEPARATOR) {
      last_slash--;
    }

    /* Reduce repetative slashes. */
    while (last_slash > path && last_slash[-1] == OS_PATH_SEPARATOR) {
      last_slash--;
    }
  }

  /* Check for the root of a drive. */
  if (os_file_is_root(path, last_slash)) {
    return (NULL);
  }

  /* Non-trivial directory component */

  return (mem_strdupl(path, last_slash - path));
}
