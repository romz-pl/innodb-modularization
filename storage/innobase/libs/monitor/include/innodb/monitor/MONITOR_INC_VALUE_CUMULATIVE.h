#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_CHECK_DEFINED.h>
#include <innodb/monitor/MONITOR_IS_ON.h>
#include <innodb/monitor/MONITOR_VALUE.h>
#include <innodb/monitor/MONITOR_MAX_VALUE.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

/** This macro updates 3 counters in one call. However, it only checks the
main/first monitor counter 'monitor', to see it is on or off to decide
whether to do the update.
@param monitor the main monitor counter to update. It accounts for
                        the accumulative value for the counter.
@param monitor_n_calls counter that counts number of times this macro is
                        called
@param monitor_per_call counter that records the current and max value of
                        each incremental value
@param value incremental value to record this time */
#define MONITOR_INC_VALUE_CUMULATIVE(monitor, monitor_n_calls,   \
                                     monitor_per_call, value)    \
  MONITOR_CHECK_DEFINED(value);                                  \
  if (MONITOR_IS_ON(monitor)) {                                  \
    MONITOR_VALUE(monitor_n_calls)++;                            \
    MONITOR_VALUE(monitor_per_call) = (mon_type_t)(value);       \
    if (MONITOR_VALUE(monitor_per_call) >                        \
        MONITOR_MAX_VALUE(monitor_per_call)) {                   \
      MONITOR_MAX_VALUE(monitor_per_call) = (mon_type_t)(value); \
    }                                                            \
    MONITOR_VALUE(monitor) += (mon_type_t)(value);               \
    if (MONITOR_VALUE(monitor) > MONITOR_MAX_VALUE(monitor)) {   \
      MONITOR_MAX_VALUE(monitor) = MONITOR_VALUE(monitor);       \
    }                                                            \
  }


#endif
