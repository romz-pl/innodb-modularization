#include <innodb/io/os_file_create_simple_func.h>

#include <innodb/assert/assert.h>
#include <innodb/error/dberr_t.h>
#include <innodb/io/access_type.h>
#include <innodb/io/os_file_create_subdirs_if_needed.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/os_file_handle_error.h>
#include <innodb/io/os_file_lock.h>
#include <innodb/io/os_innodb_umask.h>
#include <innodb/logger/error.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/** NOTE! Use the corresponding macro os_file_create_simple(), not directly
this function!
A simple function to open or create a file.
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	access_type	OS_FILE_READ_ONLY or OS_FILE_READ_WRITE
@param[in]	read_only	if true, read only checks are enforced
@param[out]	success		true if succeed, false if error
@return handle to the file, not defined if error, error number
        can be retrieved with os_file_get_last_error */
os_file_t os_file_create_simple_func(const char *name, ulint create_mode,
                                     ulint access_type, bool read_only,
                                     bool *success) {
  os_file_t file;

  *success = false;

  int create_flag;

  ut_a(!(create_mode & OS_FILE_ON_ERROR_SILENT));
  ut_a(!(create_mode & OS_FILE_ON_ERROR_NO_EXIT));

  if (create_mode == OS_FILE_OPEN) {
    if (access_type == OS_FILE_READ_ONLY) {
      create_flag = O_RDONLY;

    } else if (read_only) {
      create_flag = O_RDONLY;

    } else {
      create_flag = O_RDWR;
    }

  } else if (read_only) {
    create_flag = O_RDONLY;

  } else if (create_mode == OS_FILE_CREATE) {
    create_flag = O_RDWR | O_CREAT | O_EXCL;

  } else if (create_mode == OS_FILE_CREATE_PATH) {
    /* Create subdirs along the path if needed. */
    dberr_t err;

    err = os_file_create_subdirs_if_needed(name);

    if (err != DB_SUCCESS) {
      *success = false;
      ib::error(ER_IB_MSG_776)
          << "Unable to create subdirectories '" << name << "'";

      return (OS_FILE_CLOSED);
    }

    create_flag = O_RDWR | O_CREAT | O_EXCL;
    create_mode = OS_FILE_CREATE;
  } else {
    ib::error(ER_IB_MSG_777) << "Unknown file create mode (" << create_mode
                             << " for file '" << name << "'";

    return (OS_FILE_CLOSED);
  }

  bool retry;

  do {
    file = ::open(name, create_flag, os_innodb_umask);

    if (file == -1) {
      *success = false;

      retry = os_file_handle_error(
          name, create_mode == OS_FILE_OPEN ? "open" : "create");
    } else {
      *success = true;
      retry = false;
    }

  } while (retry);

#ifdef USE_FILE_LOCK
  if (!read_only && *success && access_type == OS_FILE_READ_WRITE &&
      os_file_lock(file, name)) {
    *success = false;
    close(file);
    file = -1;
  }
#endif /* USE_FILE_LOCK */

  return (file);
}
