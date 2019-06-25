#pragma once

#include <innodb/univ/univ.h>
#include <sys/time.h>

#ifdef _WIN32
/** This is the Windows version of gettimeofday(2).
 @return 0 if all OK else -1 */
static int ut_gettimeofday(
    struct timeval *tv, /*!< out: Values are relative to Unix epoch */
    void *tz)           /*!< in: not used */
{
  FILETIME ft;
  int64_t tm;

  if (!tv) {
    errno = EINVAL;
    return (-1);
  }

  ut_get_system_time_as_file_time(&ft);

  tm = (int64_t)ft.dwHighDateTime << 32;
  tm |= ft.dwLowDateTime;

  ut_a(tm >= 0); /* If tm wraps over to negative, the quotient / 10
                 does not work */

  tm /= 10; /* Convert from 100 nsec periods to usec */

  /* If we don't convert to the Unix epoch the value for
  struct timeval::tv_sec will overflow.*/
  tm -= WIN_TO_UNIX_DELTA_USEC;

  tv->tv_sec = (long)(tm / 1000000L);
  tv->tv_usec = (long)(tm % 1000000L);

  return (0);
}
#else
/** An alias for gettimeofday(2).  On Microsoft Windows, we have to
reimplement this function. */
#define ut_gettimeofday gettimeofday
#endif
