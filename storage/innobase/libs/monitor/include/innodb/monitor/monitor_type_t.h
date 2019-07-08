#pragma once

#include <innodb/univ/univ.h>

/** Follwoing defines are possible values for "monitor_type" field in
"struct monitor_info" */
enum monitor_type_t {
  MONITOR_NONE = 0,            /*!< No monitoring */
  MONITOR_MODULE = 1,          /*!< This is a monitor module type,
                               not a counter */
  MONITOR_EXISTING = 2,        /*!< The monitor carries information from
                               an existing system status variable */
  MONITOR_NO_AVERAGE = 4,      /*!< Set this status if we don't want to
                               calculate the average value for the counter */
  MONITOR_DISPLAY_CURRENT = 8, /*!< Display current value of the
                               counter, rather than incremental value
                               over the period. Mostly for counters
                               displaying current resource usage */
  MONITOR_GROUP_MODULE = 16,   /*!< Monitor can be turned on/off
                               only as a module, but not individually */
  MONITOR_DEFAULT_ON = 32,     /*!< Monitor will be turned on by default at
                               server start up */
  MONITOR_SET_OWNER = 64,      /*!< Owner of "monitor set", a set of
                               monitor counters */
  MONITOR_SET_MEMBER = 128,    /*!< Being part of a "monitor set" */
  MONITOR_HIDDEN = 256         /*!< Do not display this monitor in the
                               metrics table */
};
