#pragma once

#include <innodb/univ/univ.h>

#include <innodb/monitor/MONITOR_INC.h>
#include <innodb/monitor/MONITOR_INC_VALUE.h>

#define MONITOR_INC_WAIT_STATS_EX(monitor_prefix, monitor_sufix, wait_stats) \
  if ((wait_stats).wait_loops == 0) {                                        \
    MONITOR_INC(monitor_prefix##NO_WAITS##monitor_sufix);                    \
  } else {                                                                   \
    MONITOR_INC(monitor_prefix##WAITS##monitor_sufix);                       \
    MONITOR_INC_VALUE(monitor_prefix##WAIT_LOOPS##monitor_sufix,             \
                      (wait_stats).wait_loops);                              \
  }
