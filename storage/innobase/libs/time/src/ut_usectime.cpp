#include <innodb/time/ut_usectime.h>
#include <innodb/logger/error.h>
#include <innodb/time/ut_gettimeofday.h>

/** Returns system time.
 Upon successful completion, the value 0 is returned; otherwise the
 value -1 is returned and the global variable errno is set to indicate the
 error.
 @return 0 on success, -1 otherwise */
int ut_usectime(ulint *sec, /*!< out: seconds since the Epoch */
                ulint *ms)  /*!< out: microseconds since the Epoch+*sec */
{
  struct timeval tv;
  int ret = 0;
  int errno_gettimeofday;
  int i;

  for (i = 0; i < 10; i++) {
    ret = ut_gettimeofday(&tv, NULL);

    if (ret == -1) {
      errno_gettimeofday = errno;

#ifdef UNIV_NO_ERR_MSGS
      ib::error()
#else
      ib::error(ER_IB_MSG_1213)
#endif /* UNIV_NO_ERR_MSGS */
          << "gettimeofday(): " << strerror(errno_gettimeofday);

      os_thread_sleep(100000); /* 0.1 sec */
      errno = errno_gettimeofday;
    } else {
      break;
    }
  }

  if (ret != -1) {
    *sec = (ulint)tv.tv_sec;
    *ms = (ulint)tv.tv_usec;
  }

  return (ret);
}
