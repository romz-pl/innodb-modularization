#include <innodb/io/os_file_create_directory.h>

#include <innodb/io/os_file_handle_error_no_exit.h>

#include <sys/stat.h>
#include <sys/types.h>

/** This function attempts to create a directory named pathname. The new
directory gets default permissions. On Unix the permissions are
(0770 & ~umask). If the directory exists already, nothing is done and
the call succeeds, unless the fail_if_exists arguments is true.
If another error occurs, such as a permission error, this does not crash,
but reports the error and returns false.
@param[in]	pathname	directory name as null-terminated string
@param[in]	fail_if_exists	if true, pre-existing directory is treated as
                                an error.
@return true if call succeeds, false on error */
bool os_file_create_directory(const char *pathname, bool fail_if_exists) {
  int rcode = mkdir(pathname, 0770);

  if (!(rcode == 0 || (errno == EEXIST && !fail_if_exists))) {
    /* failure */
    os_file_handle_error_no_exit(pathname, "mkdir", false);

    return (false);
  }

  return (true);
}
