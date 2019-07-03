#include <innodb/io/os_file_create_simple_no_error_handling_func.h>

#include <innodb/assert/assert.h>
#include <innodb/error/dberr_t.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/os_file_lock.h>
#include <innodb/io/os_innodb_umask.h>
#include <innodb/logger/error.h>
#include <innodb/io/access_type.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/** NOTE! Use the corresponding macro
os_file_create_simple_no_error_handling(), not directly this function!
A simple function to open or create a file.
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	access_type	OS_FILE_READ_ONLY, OS_FILE_READ_WRITE, or
                                OS_FILE_READ_ALLOW_DELETE; the last option
                                is used by a backup program reading the file
@param[in]	read_only	if true read only mode checks are enforced
@param[out]	success		true if succeeded
@return own: handle to the file, not defined if error, error number
        can be retrieved with os_file_get_last_error */
pfs_os_file_t os_file_create_simple_no_error_handling_func(const char *name,
                                                           ulint create_mode,
                                                           ulint access_type,
                                                           bool read_only,
                                                           bool *success) {
  pfs_os_file_t file;
  int create_flag;

  ut_a(!(create_mode & OS_FILE_ON_ERROR_SILENT));
  ut_a(!(create_mode & OS_FILE_ON_ERROR_NO_EXIT));

  *success = false;

  if (create_mode == OS_FILE_OPEN) {
    if (access_type == OS_FILE_READ_ONLY) {
      create_flag = O_RDONLY;

    } else if (read_only) {
      create_flag = O_RDONLY;

    } else {
      ut_a(access_type == OS_FILE_READ_WRITE ||
           access_type == OS_FILE_READ_ALLOW_DELETE);

      create_flag = O_RDWR;
    }

  } else if (read_only) {
    create_flag = O_RDONLY;

  } else if (create_mode == OS_FILE_CREATE) {
    create_flag = O_RDWR | O_CREAT | O_EXCL;

  } else {
    ib::error(ER_IB_MSG_782) << "Unknown file create mode " << create_mode
                             << " for file '" << name << "'";
    file.m_file = OS_FILE_CLOSED;
    return (file);
  }

  file.m_file = ::open(name, create_flag, os_innodb_umask);

  *success = (file.m_file != -1);

#ifdef USE_FILE_LOCK
  if (!read_only && *success && access_type == OS_FILE_READ_WRITE &&
      os_file_lock(file.m_file, name)) {
    *success = false;
    close(file.m_file);
    file.m_file = -1;
  }
#endif /* USE_FILE_LOCK */

  return (file);
}
