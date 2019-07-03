#include <innodb/io/os_file_handle_error_no_exit.h>

#include <innodb/io/os_file_handle_error_cond_exit.h>

/** Does error handling when a file operation fails.
@param[in]	name		name of a file or NULL
@param[in]	operation	operation name that failed
@param[in]	on_error_silent	if true then don't print any message to the log.
@return true if we should retry the operation */
bool os_file_handle_error_no_exit(const char *name,
                                         const char *operation,
                                         bool on_error_silent) {
  /* Don't exit in case of unknown error */
  return (os_file_handle_error_cond_exit(name, operation, false, on_error_silent));
}
