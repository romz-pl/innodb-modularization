#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_STATUS.h>
#include <innodb/monitor/MONITOR_MIN_VALUE.h>
#include <innodb/monitor/MONITOR_MAX_VALUE.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

/** Max and min values are initialized when we first turn on the monitor
counter, and set the MONITOR_STATUS. */
#define MONITOR_MAX_MIN_NOT_INIT(monitor)                   \
  (MONITOR_STATUS(monitor) == MONITOR_INIT_ZERO_VALUE &&    \
   MONITOR_MIN_VALUE(monitor) == MONITOR_INIT_ZERO_VALUE && \
   MONITOR_MAX_VALUE(monitor) == MONITOR_INIT_ZERO_VALUE)


#endif
