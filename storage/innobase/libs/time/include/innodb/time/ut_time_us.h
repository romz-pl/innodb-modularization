#pragma once

#include <innodb/univ/univ.h>


/** Returns the number of microseconds since epoch. Similar to
 time(3), the return value is also stored in *tloc, provided
 that tloc is non-NULL.
 @return us since epoch */
uintmax_t ut_time_us(uintmax_t *tloc); /*!< out: us since epoch, if non-NULL */
