#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_VALUE.h>
#include <innodb/monitor/MONITOR_MAX_VALUE.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

/* Increment/decrement counter without check the monitor on/off bit, which
could already be checked as a module group */
#define MONITOR_INC_NOCHECK(monitor)                           \
  do {                                                         \
    MONITOR_VALUE(monitor)++;                                  \
    if (MONITOR_VALUE(monitor) > MONITOR_MAX_VALUE(monitor)) { \
      MONITOR_MAX_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
  } while (0)


#endif
