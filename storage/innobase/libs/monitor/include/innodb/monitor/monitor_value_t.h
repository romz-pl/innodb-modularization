#pragma once

#include <innodb/univ/univ.h>

#include <innodb/time/ib_time_t.h>
#include <innodb/monitor/mon_type_t.h>
#include <innodb/monitor/monitor_running_t.h>

/** Two monitor structures are defined in this file. One is
"monitor_value_t" which contains dynamic counter values for each
counter. The other is "monitor_info_t", which contains
static information (counter name, desc etc.) for each counter.
In addition, an enum datatype "monitor_id_t" is also defined,
it identifies each monitor with an internally used symbol, whose
integer value indexes into above two structure for its dynamic
and static information.
Developer who intend to add new counters would require to
fill in counter information as described in "monitor_info_t" and
create the internal counter ID in "monitor_id_t". */


/** Structure containing the actual values of a monitor counter. */
struct monitor_value_t {
  ib_time_t mon_start_time;       /*!< Start time of monitoring  */
  ib_time_t mon_stop_time;        /*!< Stop time of monitoring */
  ib_time_t mon_reset_time;       /*!< Time counter resetted */
  mon_type_t mon_value;           /*!< Current counter Value */
  mon_type_t mon_max_value;       /*!< Current Max value */
  mon_type_t mon_min_value;       /*!< Current Min value */
  mon_type_t mon_value_reset;     /*!< value at last reset */
  mon_type_t mon_max_value_start; /*!< Max value since start */
  mon_type_t mon_min_value_start; /*!< Min value since start */
  mon_type_t mon_start_value;     /*!< Value at the start time */
  mon_type_t mon_last_value;      /*!< Last set of values */
  monitor_running_t mon_status;   /* whether monitor still running */
};
