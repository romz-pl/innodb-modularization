#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_CHECK_DEFINED.h>
#include <innodb/monitor/MONITOR_IS_ON.h>
#include <innodb/monitor/MONITOR_VALUE.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

/** Some values such as log sequence number are montomically increasing
number, do not need to record max/min values */
#define MONITOR_SET_SIMPLE(monitor, value)        \
  MONITOR_CHECK_DEFINED(value);                   \
  if (MONITOR_IS_ON(monitor)) {                   \
    MONITOR_VALUE(monitor) = (mon_type_t)(value); \
  }


#endif
