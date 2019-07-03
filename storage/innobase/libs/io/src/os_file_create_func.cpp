#include <innodb/io/os_file_create_func.h>

#include <innodb/io/os_file_create_subdirs_if_needed.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/os_file_handle_error_no_exit.h>
#include <innodb/io/os_file_handle_error.h>
#include <innodb/io/os_file_set_nocache.h>
#include <innodb/io/os_innodb_umask.h>
#include <innodb/io/srv_unix_flush_t.h>
#include <innodb/logger/error.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern enum srv_unix_flush_t srv_unix_file_flush_method;

/** NOTE! Use the corresponding macro os_file_create(), not directly
this function!
Opens an existing file or creates a new.
@param[in]	name		name of the file or path as a null-terminated
                                string
@param[in]	create_mode	create mode
@param[in]	purpose		OS_FILE_AIO, if asynchronous, non-buffered I/O
                                is desired, OS_FILE_NORMAL, if any normal file;
                                NOTE that it also depends on type, os_aio_..
                                and srv_.. variables whether we really use async
                                I/O or unbuffered I/O: look in the function
                                source code for the exact rules
@param[in]	type		OS_DATA_FILE or OS_LOG_FILE
@param[in]	read_only	true, if read only checks should be enforcedm
@param[in]	success		true if succeeded
@return handle to the file, not defined if error, error number
        can be retrieved with os_file_get_last_error */
pfs_os_file_t os_file_create_func(const char *name, ulint create_mode,
                                  ulint purpose, ulint type, bool read_only,
                                  bool *success) {
  bool on_error_no_exit;
  bool on_error_silent;
  pfs_os_file_t file;

  *success = false;

  DBUG_EXECUTE_IF("ib_create_table_fail_disk_full", *success = false;
                  errno = ENOSPC; file.m_file = OS_FILE_CLOSED; return (file););

  int create_flag;
  const char *mode_str = NULL;

  on_error_no_exit = create_mode & OS_FILE_ON_ERROR_NO_EXIT ? true : false;
  on_error_silent = create_mode & OS_FILE_ON_ERROR_SILENT ? true : false;

  create_mode &= ~OS_FILE_ON_ERROR_NO_EXIT;
  create_mode &= ~OS_FILE_ON_ERROR_SILENT;

  if (create_mode == OS_FILE_OPEN || create_mode == OS_FILE_OPEN_RAW ||
      create_mode == OS_FILE_OPEN_RETRY) {
    mode_str = "OPEN";

    create_flag = read_only ? O_RDONLY : O_RDWR;

  } else if (read_only) {
    mode_str = "OPEN";

    create_flag = O_RDONLY;

  } else if (create_mode == OS_FILE_CREATE) {
    mode_str = "CREATE";
    create_flag = O_RDWR | O_CREAT | O_EXCL;

  } else if (create_mode == OS_FILE_CREATE_PATH) {
    /* Create subdirs along the path if needed. */
    dberr_t err;

    err = os_file_create_subdirs_if_needed(name);

    if (err != DB_SUCCESS) {
      *success = false;
      ib::error(ER_IB_MSG_778)
          << "Unable to create subdirectories '" << name << "'";

      file.m_file = OS_FILE_CLOSED;
      return (file);
    }

    create_flag = O_RDWR | O_CREAT | O_EXCL;
    create_mode = OS_FILE_CREATE;

  } else {
    ib::error(ER_IB_MSG_779)
        << "Unknown file create mode (" << create_mode << ")"
        << " for file '" << name << "'";

    file.m_file = OS_FILE_CLOSED;
    return (file);
  }

  ut_a(type == OS_LOG_FILE || type == OS_DATA_FILE ||
       type == OS_CLONE_DATA_FILE || type == OS_CLONE_LOG_FILE ||
       type == OS_BUFFERED_FILE);

  ut_a(purpose == OS_FILE_AIO || purpose == OS_FILE_NORMAL);

#ifdef O_SYNC
  /* We let O_SYNC only affect log files; note that we map O_DSYNC to
  O_SYNC because the datasync options seemed to corrupt files in 2001
  in both Linux and Solaris */

  if (!read_only && type == OS_LOG_FILE &&
      srv_unix_file_flush_method == SRV_UNIX_O_DSYNC) {
    create_flag |= O_SYNC;
  }
#endif /* O_SYNC */

  bool retry;

  do {
    file.m_file = ::open(name, create_flag, os_innodb_umask);

    if (file.m_file == -1) {
      const char *operation;

      operation =
          (create_mode == OS_FILE_CREATE && !read_only) ? "create" : "open";

      *success = false;

      if (on_error_no_exit) {
        retry = os_file_handle_error_no_exit(name, operation, on_error_silent);
      } else {
        retry = os_file_handle_error(name, operation);
      }
    } else {
      *success = true;
      retry = false;
    }

  } while (retry);

  /* We disable OS caching (O_DIRECT) only on data files. For clone we
  need to set O_DIRECT even for read_only mode. */

  if ((!read_only || type == OS_CLONE_DATA_FILE) && *success &&
      (type == OS_DATA_FILE || type == OS_CLONE_DATA_FILE) &&
      (srv_unix_file_flush_method == SRV_UNIX_O_DIRECT ||
       srv_unix_file_flush_method == SRV_UNIX_O_DIRECT_NO_FSYNC)) {
    os_file_set_nocache(file.m_file, name, mode_str);
  }

#ifdef USE_FILE_LOCK
  if (!read_only && *success && create_mode != OS_FILE_OPEN_RAW &&
      /* Don't acquire file lock while cloning files. */
      type != OS_CLONE_DATA_FILE && type != OS_CLONE_LOG_FILE &&
      os_file_lock(file.m_file, name)) {
    if (create_mode == OS_FILE_OPEN_RETRY) {
      ib::info(ER_IB_MSG_780) << "Retrying to lock the first data file";

      for (int i = 0; i < 100; i++) {
        os_thread_sleep(1000000);

        if (!os_file_lock(file.m_file, name)) {
          *success = true;
          return (file);
        }
      }

      ib::info(ER_IB_MSG_781) << "Unable to open the first data file";
    }

    *success = false;
    close(file.m_file);
    file.m_file = -1;
  }
#endif /* USE_FILE_LOCK */

  return (file);
}
