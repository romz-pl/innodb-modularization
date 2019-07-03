#include <innodb/io/os_file_close_func.h>

#include <innodb/io/os_file_handle_error.h>

/** NOTE! Use the corresponding macro os_file_close(), not directly this
function!
Closes a file handle. In case of error, error number can be retrieved with
os_file_get_last_error.
@param[in]	file		Handle to close
@return true if success */
bool os_file_close_func(os_file_t file) {
  int ret = close(file);

  if (ret == -1) {
    os_file_handle_error(NULL, "close");

    return (false);
  }

  return (true);
}
