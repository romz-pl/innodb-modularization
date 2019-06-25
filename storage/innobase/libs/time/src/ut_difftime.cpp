#include <innodb/time/ut_difftime.h>


/** Returns the difference of two times in seconds.
 @return time2 - time1 expressed in seconds */
double ut_difftime(ib_time_t time2, /*!< in: time */
                   ib_time_t time1) /*!< in: time */
{
  return (difftime(time2, time1));
}
