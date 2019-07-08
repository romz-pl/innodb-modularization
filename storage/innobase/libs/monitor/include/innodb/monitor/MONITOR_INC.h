#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_IS_ON.h>
#include <innodb/monitor/MONITOR_VALUE.h>
#include <innodb/monitor/MONITOR_MAX_VALUE.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

/** Macros to increment/decrement the counters. The normal
monitor counter operation expects appropriate synchronization
already exists. No additional mutex is necessary when operating
on the counters */
#define MONITOR_INC(monitor)                                   \
  if (MONITOR_IS_ON(monitor)) {                                \
    MONITOR_VALUE(monitor)++;                                  \
    if (MONITOR_VALUE(monitor) > MONITOR_MAX_VALUE(monitor)) { \
      MONITOR_MAX_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
  }


#endif
