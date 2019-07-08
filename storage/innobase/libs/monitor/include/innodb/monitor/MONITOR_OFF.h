#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

/** Macros to turn on/off the control bit in monitor_set_tbl for a monitor
counter option. */
#define MONITOR_OFF(monitor)                    \
  (monitor_set_tbl[monitor / NUM_BITS_ULINT] &= \
   ~((ulint)1 << (monitor % NUM_BITS_ULINT)))


#endif
