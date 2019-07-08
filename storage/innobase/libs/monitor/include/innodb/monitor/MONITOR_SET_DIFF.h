#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_SET_UPD_MAX_ONLY.h>
#include <innodb/monitor/MONITOR_VALUE_RESET.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Following four macros defines necessary operations to fetch and
consolidate information from existing system status variables. */


/** Set monitor value to the difference of value and mon_start_value
compensated by mon_last_value if accumulated value is required. */
#define MONITOR_SET_DIFF(monitor, value)                                       \
  MONITOR_SET_UPD_MAX_ONLY(monitor, ((value)-MONITOR_VALUE_RESET(monitor) -    \
                                     MONITOR_FIELD(monitor, mon_start_value) + \
                                     MONITOR_FIELD(monitor, mon_last_value)))



#endif
