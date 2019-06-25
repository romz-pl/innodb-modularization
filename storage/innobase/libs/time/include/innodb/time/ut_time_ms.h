#pragma once

#include <innodb/univ/univ.h>

/** Returns the number of milliseconds since some epoch.  The
 value may wrap around.  It should only be used for heuristic
 purposes.
 @return ms since epoch */
ulint ut_time_ms(void);
