#include <innodb/io/os_file_handle_error_cond_exit.h>

#include <innodb/io/os_file_get_last_error_low.h>
#include <innodb/logger/error.h>
#include <innodb/logger/fatal.h>
#include <innodb/thread/os_thread_sleep.h>
#include <innodb/io/os_has_said_disk_full.h>

void srv_fatal_error();



/** Does error handling when a file operation fails.
Conditionally exits (calling srv_fatal_error()) based on should_exit value
and the error type, if should_exit is true then on_error_silent is ignored.
@param[in]	name		name of a file or NULL
@param[in]	operation	operation
@param[in]	should_exit	call srv_fatal_error() on an unknown error,
                                if this parameter is true
@param[in]	on_error_silent	if true then don't print any message to the log
                                iff it is an unknown non-fatal error
@return true if we should retry the operation */
MY_ATTRIBUTE((warn_unused_result)) bool os_file_handle_error_cond_exit(
    const char *name, const char *operation, bool should_exit,
    bool on_error_silent) {
  ulint err;

  err = os_file_get_last_error_low(false, on_error_silent);

  switch (err) {
    case OS_FILE_DISK_FULL:
      /* We only print a warning about disk full once */

      if (os_has_said_disk_full) {
        return (false);
      }

      /* Disk full error is reported irrespective of the
      on_error_silent setting. */

      if (name) {
        ib::error(ER_IB_MSG_819)
            << "Encountered a problem with file '" << name << "'";
      }

      ib::error(ER_IB_MSG_820)
          << "Disk is full. Try to clean the disk to free space.";

      os_has_said_disk_full = true;

      return (false);

    case OS_FILE_AIO_RESOURCES_RESERVED:
    case OS_FILE_AIO_INTERRUPTED:

      return (true);

    case OS_FILE_PATH_ERROR:
    case OS_FILE_ALREADY_EXISTS:
    case OS_FILE_ACCESS_VIOLATION:

      return (false);

    case OS_FILE_SHARING_VIOLATION:

      os_thread_sleep(10000000); /* 10 sec */
      return (true);

    case OS_FILE_OPERATION_ABORTED:
    case OS_FILE_INSUFFICIENT_RESOURCE:

      os_thread_sleep(100000); /* 100 ms */
      return (true);

    case OS_FILE_NAME_TOO_LONG:
      return (false);

    default:

      /* If it is an operation that can crash on error then it
      is better to ignore on_error_silent and print an error message
      to the log. */

      if (should_exit || !on_error_silent) {
        ib::error(ER_IB_MSG_821)
            << "File " << (name != NULL ? name : "(unknown)") << ": '"
            << operation
            << "'"
               " returned OS error "
            << err << "." << (should_exit ? " Cannot continue operation" : "");
      }

      if (should_exit) {
#ifndef UNIV_HOTBACKUP
        srv_fatal_error();
#else  /* !UNIV_HOTBACKUP */
        ib::fatal(ER_IB_MSG_822) << "Internal error,"
                                 << " cannot continue operation.";
#endif /* !UNIV_HOTBACKUP */
      }
  }

  return (false);
}
