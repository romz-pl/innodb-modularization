#pragma once

#include <innodb/univ/univ.h>

#include <innodb/monitor/MONITOR_INC_WAIT_STATS_EX.h>

#define MONITOR_INC_WAIT_STATS(monitor_prefix, wait_stats) \
  MONITOR_INC_WAIT_STATS_EX(monitor_prefix, , wait_stats);
