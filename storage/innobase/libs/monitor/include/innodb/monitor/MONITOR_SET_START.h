#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_FIELD.h>
#include <innodb/monitor/MONITOR_STATUS.h>

#include <time.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

#define MONITOR_SET_START(monitor)                         \
  do {                                                     \
    MONITOR_STATUS(monitor) = MONITOR_STARTED;             \
    MONITOR_FIELD((monitor), mon_start_time) = time(NULL); \
  } while (0)


#endif
