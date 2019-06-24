#pragma once

#include <time.h>
/** Time stamp */
typedef time_t ib_time_t;

/** If we are doing something that takes longer than this many seconds then
print an informative message. Type should be return type of ut_time(). */
static constexpr ib_time_t PRINT_INTERVAL_SECS = 10;
