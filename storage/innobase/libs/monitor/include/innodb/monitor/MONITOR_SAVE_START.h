#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_CHECK_DEFINED.h>
#include <innodb/monitor/MONITOR_START_VALUE.h>
#include <innodb/monitor/MONITOR_VALUE_RESET.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Following four macros defines necessary operations to fetch and
consolidate information from existing system status variables. */

/** Save the passed-in value to mon_start_value field of monitor
counters */
#define MONITOR_SAVE_START(monitor, value)                  \
  do {                                                      \
    MONITOR_CHECK_DEFINED(value);                           \
    (MONITOR_START_VALUE(monitor) =                         \
         (mon_type_t)(value)-MONITOR_VALUE_RESET(monitor)); \
  } while (0)



#endif
