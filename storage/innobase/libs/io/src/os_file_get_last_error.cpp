#include <innodb/io/os_file_get_last_error.h>

#include <innodb/io/os_file_get_last_error_low.h>

/** Retrieves the last error number if an error occurs in a file io function.
The number should be retrieved before any other OS calls (because they may
overwrite the error number). If the number is not known to this program,
the OS error number + 100 is returned.
@param[in]	report_all_errors	true if we want an error printed
                                        for all errors
@return error number, or OS error number + 100 */
ulint os_file_get_last_error(bool report_all_errors) {
  return (os_file_get_last_error_low(report_all_errors, false));
}
