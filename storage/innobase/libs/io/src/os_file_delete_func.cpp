#include <innodb/io/os_file_delete_func.h>

#include <innodb/io/os_file_handle_error_no_exit.h>

#include <unistd.h>

/** Deletes a file. The file has to be closed before calling this.
@param[in]	name		file path as a null-terminated string
@return true if success */
bool os_file_delete_func(const char *name) {
  int ret = unlink(name);

  if (ret != 0) {
    os_file_handle_error_no_exit(name, "delete", false);

    return (false);
  }

  return (true);
}
