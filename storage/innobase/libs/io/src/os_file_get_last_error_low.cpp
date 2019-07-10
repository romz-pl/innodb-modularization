#include <innodb/io/os_file_get_last_error_low.h>

#include <innodb/logger/error.h>
#include <innodb/logger/info.h>
#include <innodb/io/srv_use_native_aio.h>
#include <innodb/io/srv_is_being_started.h>
#include <innodb/io/OPERATING_SYSTEM_ERROR_MSG.h>

#include <string.h>

/** Retrieves the last error number if an error occurs in a file io function.
The number should be retrieved before any other OS calls (because they may
overwrite the error number). If the number is not known to this program,
the OS error number + 100 is returned.
@param[in]	report_all_errors	true if we want an error message
                                        printed of all errors
@param[in]	on_error_silent		true then don't print any diagnostic
                                        to the log
@return error number, or OS error number + 100 */
ulint os_file_get_last_error_low(bool report_all_errors,
                                        bool on_error_silent) {
  int err = errno;

  if (err == 0) {
    return (0);
  }

  if (report_all_errors ||
      (err != ENOSPC && err != EEXIST && !on_error_silent)) {
    ib::error(ER_IB_MSG_767)
        << "Operating system error number " << err << " in a file operation.";

    if (err == ENOENT) {
      ib::error(ER_IB_MSG_768) << "The error means the system"
                                  " cannot find the path specified.";

#ifndef UNIV_HOTBACKUP
      if (srv_is_being_started) {
        ib::error(ER_IB_MSG_769) << "If you are installing InnoDB,"
                                    " remember that you must create"
                                    " directories yourself, InnoDB"
                                    " does not create them.";
      }
#endif /* !UNIV_HOTBACKUP */
    } else if (err == EACCES) {
      ib::error(ER_IB_MSG_770) << "The error means mysqld does not have"
                                  " the access rights to the directory.";

    } else {
      if (strerror(err) != NULL) {
        ib::error(ER_IB_MSG_771)
            << "Error number " << err << " means '" << strerror(err) << "'";
      }

      ib::info(ER_IB_MSG_772) << OPERATING_SYSTEM_ERROR_MSG;
    }
  }

  switch (err) {
    case ENOSPC:
      return (OS_FILE_DISK_FULL);
    case ENOENT:
      return (OS_FILE_NOT_FOUND);
    case EEXIST:
      return (OS_FILE_ALREADY_EXISTS);
    case EXDEV:
    case ENOTDIR:
    case EISDIR:
      return (OS_FILE_PATH_ERROR);
    case EAGAIN:
      if (srv_use_native_aio) {
        return (OS_FILE_AIO_RESOURCES_RESERVED);
      }
      break;
    case EINTR:
      if (srv_use_native_aio) {
        return (OS_FILE_AIO_INTERRUPTED);
      }
      break;
    case EACCES:
      return (OS_FILE_ACCESS_VIOLATION);
    case ENAMETOOLONG:
      return (OS_FILE_NAME_TOO_LONG);
  }
  return (OS_FILE_ERROR_MAX + err);
}
