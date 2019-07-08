#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_MAX_MIN_NOT_INIT.h>
#include <innodb/monitor/MONITOR_MIN_VALUE_START.h>
#include <innodb/monitor/MONITOR_MAX_VALUE_START.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

#define MONITOR_INIT(monitor)                        \
  if (MONITOR_MAX_MIN_NOT_INIT(monitor)) {           \
    MONITOR_MIN_VALUE(monitor) = MIN_RESERVED;       \
    MONITOR_MIN_VALUE_START(monitor) = MIN_RESERVED; \
    MONITOR_MAX_VALUE(monitor) = MAX_RESERVED;       \
    MONITOR_MAX_VALUE_START(monitor) = MAX_RESERVED; \
  }


#endif
