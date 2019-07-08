#pragma once

#include <innodb/univ/univ.h>

/** Following are the "set_option" values allowed for
srv_mon_process_existing_counter() and srv_mon_process_existing_counter()
functions. To turn on/off/reset the monitor counters. */
enum mon_option_t {
  MONITOR_TURN_ON = 1,     /*!< Turn on the counter */
  MONITOR_TURN_OFF,        /*!< Turn off the counter */
  MONITOR_RESET_VALUE,     /*!< Reset current values */
  MONITOR_RESET_ALL_VALUE, /*!< Reset all values */
  MONITOR_GET_VALUE        /*!< Option for
                           srv_mon_process_existing_counter()
                           function */
};
