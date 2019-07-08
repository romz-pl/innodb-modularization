#pragma once

#include <innodb/univ/univ.h>

/** Possible status values for "mon_status" in "struct monitor_value" */
enum monitor_running_status {
  MONITOR_STARTED = 1, /*!< Monitor has been turned on */
  MONITOR_STOPPED = 2  /*!< Monitor has been turned off */
};


