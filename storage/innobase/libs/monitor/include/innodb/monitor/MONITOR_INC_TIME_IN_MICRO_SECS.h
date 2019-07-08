#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_CHECK_DEFINED.h>
#include <innodb/monitor/MONITOR_IS_ON.h>
#include <innodb/monitor/MONITOR_VALUE.h>
#include <innodb/time/ut_time_us.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

/** Add time difference between now and input "value" (in seconds) to the
monitor counter
@param monitor monitor to update for the time difference
@param value the start time value */
#define MONITOR_INC_TIME_IN_MICRO_SECS(monitor, value)        \
  MONITOR_CHECK_DEFINED(value);                               \
  if (MONITOR_IS_ON(monitor)) {                               \
    uintmax_t old_time = (value);                             \
    value = ut_time_us(NULL);                                 \
    MONITOR_VALUE(monitor) += (mon_type_t)(value - old_time); \
  }


#endif
