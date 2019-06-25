#include <innodb/time/ut_time_ms.h>
#include <innodb/time/ut_gettimeofday.h>

/** Returns the number of milliseconds since some epoch.  The
 value may wrap around.  It should only be used for heuristic
 purposes.
 @return ms since epoch */
ulint ut_time_ms(void) {
  struct timeval tv;

  ut_gettimeofday(&tv, NULL);

  return ((ulint)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
