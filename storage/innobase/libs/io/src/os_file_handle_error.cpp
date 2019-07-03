#include <innodb/io/os_file_handle_error.h>

#include <innodb/io/os_file_handle_error_cond_exit.h>

/** Does error handling when a file operation fails.
@param[in]	name		name of a file or NULL
@param[in]	operation	operation name that failed
@return true if we should retry the operation */
bool os_file_handle_error(const char *name, const char *operation) {
  /* Exit in case of unknown error */
  return (os_file_handle_error_cond_exit(name, operation, true, false));
}
