#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/MONITOR_VALUE.h>
#include <innodb/monitor/MONITOR_VALUE_RESET.h>

/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

#define MONITOR_VALUE_SINCE_START(monitor) \
  (MONITOR_VALUE(monitor) + MONITOR_VALUE_RESET(monitor))


#endif
