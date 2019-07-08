#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/macros.h>
#include <innodb/monitor/monitor_set_tbl.h>

/** Macros to turn on/off the control bit in monitor_set_tbl for a monitor
counter option. */

/** Check whether the requested monitor is turned on/off */
#define MONITOR_IS_ON(monitor)                 \
  (monitor_set_tbl[monitor / NUM_BITS_ULINT] & \
   ((ulint)1 << (monitor % NUM_BITS_ULINT)))

#endif
