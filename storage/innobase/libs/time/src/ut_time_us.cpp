#include <innodb/time/ut_time_us.h>
#include <innodb/time/ut_gettimeofday.h>

/** Returns the number of microseconds since epoch. Similar to
 time(3), the return value is also stored in *tloc, provided
 that tloc is non-NULL.
 @return us since epoch */
uintmax_t ut_time_us(uintmax_t *tloc) /*!< out: us since epoch, if non-NULL */
{
  struct timeval tv;
  uintmax_t us;

  ut_gettimeofday(&tv, NULL);

  us = static_cast<uintmax_t>(tv.tv_sec) * 1000000 + tv.tv_usec;

  if (tloc != NULL) {
    *tloc = us;
  }

  return (us);
}
