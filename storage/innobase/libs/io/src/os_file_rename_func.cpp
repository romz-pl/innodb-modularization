#include <innodb/io/os_file_rename_func.h>

#include <innodb/io/os_file_handle_error_no_exit.h>

/** NOTE! Use the corresponding macro os_file_rename(), not directly this
function!
Renames a file (can also move it to another directory). It is safest that the
file is closed before calling this function.
@param[in]	oldpath		old file path as a null-terminated string
@param[in]	newpath		new file path
@return true if success */
bool os_file_rename_func(const char *oldpath, const char *newpath) {
#ifdef UNIV_DEBUG
  os_file_type_t type;
  bool exists;

  /* New path must not exist. */
  ut_ad(os_file_status(newpath, &exists, &type));
  ut_ad(!exists);

  /* Old path must exist. */
  ut_ad(os_file_status(oldpath, &exists, &type));
  ut_ad(exists);
#endif /* UNIV_DEBUG */

  int ret = rename(oldpath, newpath);

  if (ret != 0) {
    os_file_handle_error_no_exit(oldpath, "rename", false);

    return (false);
  }

  return (true);
}
