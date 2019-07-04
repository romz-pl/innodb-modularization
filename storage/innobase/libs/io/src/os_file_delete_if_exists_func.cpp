#include <innodb/io/os_file_delete_if_exists_func.h>

#include <innodb/io/os_file_can_delete.h>
#include <innodb/io/os_file_handle_error_no_exit.h>

#include <unistd.h>

/** Deletes a file if it exists. The file has to be closed before calling this.
@param[in]	name		file path as a null-terminated string
@param[out]	exist		indicate if file pre-exist
@return true if success */
bool os_file_delete_if_exists_func(const char *name, bool *exist) {
  if (!os_file_can_delete(name)) {
    return (false);
  }

  if (exist != nullptr) {
    *exist = true;
  }

  int ret = unlink(name);

  if (ret != 0 && errno == ENOENT) {
    if (exist != nullptr) {
      *exist = false;
    }

  } else if (ret != 0 && errno != ENOENT) {
    os_file_handle_error_no_exit(name, "delete", false);

    return (false);
  }

  return (true);
}

