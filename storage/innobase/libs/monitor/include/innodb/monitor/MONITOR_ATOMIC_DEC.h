#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_IS_ON.h>
#include <innodb/monitor/MONITOR_VALUE.h>
#include <innodb/monitor/MONITOR_MIN_VALUE.h>

#include <innodb/atomic/atomic.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

/** Atomically decrement a monitor counter.
Use MONITOR_DEC if appropriate mutex protection exists.
@param monitor monitor to be decremented by 1 */
#define MONITOR_ATOMIC_DEC(monitor)                                            \
  if (MONITOR_IS_ON(monitor)) {                                                \
    ib_uint64_t value;                                                         \
    value =                                                                    \
        os_atomic_decrement_uint64((ib_uint64_t *)&MONITOR_VALUE(monitor), 1); \
    /* Note: This is not 100% accurate because of the                          \
    inherent race, we ignore it due to performance. */                         \
    if (value < (ib_uint64_t)MONITOR_MIN_VALUE(monitor)) {                     \
      MONITOR_MIN_VALUE(monitor) = value;                                      \
    }                                                                          \
  }


#endif
