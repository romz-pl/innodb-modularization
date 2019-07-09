#include <innodb/io/os_file_fsync_posix.h>

#include <innodb/assert/assert.h>
#include <innodb/logger/fatal.h>
#include <innodb/logger/warn.h>
#include <innodb/error/ut_error.h>
#include <innodb/thread/os_thread_sleep.h>
#include <innodb/io/os_n_fsyncs.h>

#include <unistd.h>

/** Wrapper to fsync(2) that retries the call on some errors.
Returns the value 0 if successful; otherwise the value -1 is returned and
the global variable errno is set to indicate the error.
@param[in]	file		open file handle
@return 0 if success, -1 otherwise */
int os_file_fsync_posix(os_file_t file) {
  ulint failures = 0;
#ifdef UNIV_HOTBACKUP
  static meb::Mutex meb_mutex;
#endif /* UNIV_HOTBACKUP */

  for (;;) {
#ifdef UNIV_HOTBACKUP
    meb_mutex.lock();
#endif /* UNIV_HOTBACKUP */
    ++os_n_fsyncs;
#ifdef UNIV_HOTBACKUP
    meb_mutex.unlock();
#endif /* UNIV_HOTBACKUP */

    int ret = fsync(file);

    if (ret == 0) {
      return (ret);
    }

    switch (errno) {
      case ENOLCK:

        ++failures;
        ut_a(failures < 1000);

        if (!(failures % 100)) {
          ib::warn(ER_IB_MSG_773) << "fsync(): "
                                  << "No locks available; retrying";
        }

        /* 0.2 sec */
        os_thread_sleep(200000);
        break;

      case EIO:

        ib::fatal() << "fsync() returned EIO, aborting.";
        break;

      case EINTR:

        ++failures;
        ut_a(failures < 2000);
        break;

      default:
        ut_error;
        break;
    }
  }

  ut_error;

  return (-1);
}
