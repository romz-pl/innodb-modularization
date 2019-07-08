#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_LAST_VALUE.h>
#include <innodb/monitor/MONITOR_VALUE.h>
#include <innodb/monitor/MONITOR_START_VALUE.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Following four macros defines necessary operations to fetch and
consolidate information from existing system status variables. */


/** Save the passed-in value to mon_last_value field of monitor
counters */
#define MONITOR_SAVE_LAST(monitor)                          \
  do {                                                      \
    MONITOR_LAST_VALUE(monitor) = MONITOR_VALUE(monitor);   \
    MONITOR_START_VALUE(monitor) += MONITOR_VALUE(monitor); \
  } while (0)


#endif
