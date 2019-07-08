#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_FIELD.h>
#include <innodb/monitor/MONITOR_VALUE.h>
#include <innodb/monitor/MONITOR_MAX_VALUE.h>
#include <innodb/monitor/MONITOR_MIN_VALUE.h>
#include <innodb/monitor/MONITOR_VALUE_RESET.h>
#include <innodb/monitor/MONITOR_MAX_VALUE_START.h>
#include <innodb/monitor/MONITOR_MIN_VALUE_START.h>
#include <innodb/monitor/MONITOR_LAST_VALUE.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

/** Reset the monitor value and max/min value to zero. The reset
operation would only be conducted when the counter is turned off */
#define MONITOR_RESET_ALL(monitor)                                    \
  do {                                                                \
    MONITOR_VALUE(monitor) = MONITOR_INIT_ZERO_VALUE;                 \
    MONITOR_MAX_VALUE(monitor) = MAX_RESERVED;                        \
    MONITOR_MIN_VALUE(monitor) = MIN_RESERVED;                        \
    MONITOR_VALUE_RESET(monitor) = MONITOR_INIT_ZERO_VALUE;           \
    MONITOR_MAX_VALUE_START(monitor) = MAX_RESERVED;                  \
    MONITOR_MIN_VALUE_START(monitor) = MIN_RESERVED;                  \
    MONITOR_LAST_VALUE(monitor) = MONITOR_INIT_ZERO_VALUE;            \
    MONITOR_FIELD(monitor, mon_start_time) = MONITOR_INIT_ZERO_VALUE; \
    MONITOR_FIELD(monitor, mon_stop_time) = MONITOR_INIT_ZERO_VALUE;  \
    MONITOR_FIELD(monitor, mon_reset_time) = MONITOR_INIT_ZERO_VALUE; \
  } while (0)


#endif
