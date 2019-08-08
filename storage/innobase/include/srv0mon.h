/***********************************************************************

Copyright (c) 2010, 2018, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2012, Facebook Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

***********************************************************************/

/** @file include/srv0mon.h
 Server monitor counter related defines

 Created 12/15/2009	Jimmy Yang
 *******************************************************/

#ifndef srv0mon_h
#define srv0mon_h

#include <innodb/univ/univ.h>
#include <innodb/time/ib_time_t.h>



#include <innodb/monitor/monitor_running_status.h>
#include <innodb/monitor/mon_type_t.h>
#include <innodb/monitor/monitor_running_t.h>
#include <innodb/monitor/monitor_value_t.h>
#include <innodb/monitor/monitor_type_t.h>
#include <innodb/monitor/macros.h>
#include <innodb/monitor/monitor_id_t.h>
#include <innodb/monitor/monitor_info_t.h>
#include <innodb/monitor/mon_option_t.h>
#include <innodb/monitor/monitor_set_tbl.h>
#include <innodb/monitor/MONITOR_ON.h>
#include <innodb/monitor/MONITOR_OFF.h>
#include <innodb/monitor/MONITOR_IS_ON.h>
#include <innodb/monitor/innodb_counter_value.h>
#include <innodb/monitor/MONITOR_FIELD.h>
#include <innodb/monitor/MONITOR_VALUE.h>
#include <innodb/monitor/MONITOR_MAX_VALUE.h>
#include <innodb/monitor/MONITOR_MIN_VALUE.h>
#include <innodb/monitor/MONITOR_VALUE_RESET.h>
#include <innodb/monitor/MONITOR_MAX_VALUE_START.h>
#include <innodb/monitor/MONITOR_MIN_VALUE_START.h>
#include <innodb/monitor/MONITOR_LAST_VALUE.h>
#include <innodb/monitor/MONITOR_START_VALUE.h>
#include <innodb/monitor/MONITOR_VALUE_SINCE_START.h>
#include <innodb/monitor/MONITOR_STATUS.h>
#include <innodb/monitor/MONITOR_SET_START.h>
#include <innodb/monitor/MONITOR_SET_OFF.h>
#include <innodb/monitor/MONITOR_MAX_MIN_NOT_INIT.h>
#include <innodb/monitor/MONITOR_INIT.h>
#include <innodb/monitor/MONITOR_INC.h>
#include <innodb/monitor/MONITOR_ATOMIC_INC.h>
#include <innodb/monitor/MONITOR_ATOMIC_DEC.h>
#include <innodb/monitor/MONITOR_DEC.h>
#include <innodb/monitor/MONITOR_CHECK_DEFINED.h>
#include <innodb/monitor/MONITOR_INC_VALUE.h>
#include <innodb/monitor/MONITOR_DEC_VALUE.h>
#include <innodb/monitor/MONITOR_INC_NOCHECK.h>
#include <innodb/monitor/MONITOR_DEC_NOCHECK.h>
#include <innodb/monitor/MONITOR_SET.h>
#include <innodb/monitor/MONITOR_INC_TIME_IN_MICRO_SECS.h>
#include <innodb/monitor/MONITOR_INC_VALUE_CUMULATIVE.h>
#include <innodb/monitor/MONITOR_SET_UPD_MAX_ONLY.h>
#include <innodb/monitor/MONITOR_SET_SIMPLE.h>
#include <innodb/monitor/MONITOR_RESET_ALL.h>
#include <innodb/monitor/MONITOR_SAVE_START.h>
#include <innodb/monitor/MONITOR_SAVE_LAST.h>
#include <innodb/monitor/MONITOR_SET_DIFF.h>
#include <innodb/monitor/MONITOR_INC_WAIT_STATS.h>
#include <innodb/monitor/MONITOR_INC_WAIT_STATS_EX.h>



#ifndef UNIV_HOTBACKUP




/** Get monitor's monitor_info_t by its monitor id (index into the
 innodb_counter_info array
 @return Point to corresponding monitor_info_t, or NULL if no such
 monitor */
monitor_info_t *srv_mon_get_info(
    monitor_id_t monitor_id); /*!< id index into the
                              innodb_counter_info array */
/** Get monitor's name by its monitor id (index into the
 innodb_counter_info array
 @return corresponding monitor name, or NULL if no such
 monitor */
const char *srv_mon_get_name(
    monitor_id_t monitor_id); /*!< id index into the
                              innodb_counter_info array */

/** Turn on/off/reset monitor counters in a module. If module_value
 is NUM_MONITOR then turn on all monitor counters. */
void srv_mon_set_module_control(
    monitor_id_t module_id,   /*!< in: Module ID as in
                              monitor_counter_id. If it is
                              set to NUM_MONITOR, this means
                              we shall turn on all the counters */
    mon_option_t set_option); /*!< in: Turn on/off reset the
                              counter */
/** This function consolidates some existing server counters used
 by "system status variables". These existing system variables do not have
 mechanism to start/stop and reset the counters, so we simulate these
 controls by remembering the corresponding counter values when the
 corresponding monitors are turned on/off/reset, and do appropriate
 mathematics to deduct the actual value. */
void srv_mon_process_existing_counter(
    monitor_id_t monitor_id,  /*!< in: the monitor's ID as in
                              monitor_counter_id */
    mon_option_t set_option); /*!< in: Turn on/off reset the
                              counter */
/** This function is used to calculate the maximum counter value
 since the start of monitor counter
 @return max counter value since start. */
UNIV_INLINE
mon_type_t srv_mon_calc_max_since_start(
    monitor_id_t monitor); /*!< in: monitor id */
/** This function is used to calculate the minimum counter value
 since the start of monitor counter
 @return min counter value since start. */
UNIV_INLINE
mon_type_t srv_mon_calc_min_since_start(
    monitor_id_t monitor); /*!< in: monitor id*/
/** Reset a monitor, create a new base line with the current monitor
 value. This baseline is recorded by MONITOR_VALUE_RESET(monitor) */
void srv_mon_reset(monitor_id_t monitor); /*!< in: monitor id*/
/** This function resets all values of a monitor counter */
UNIV_INLINE
void srv_mon_reset_all(monitor_id_t monitor); /*!< in: monitor id*/
/** Turn on monitor counters that are marked as default ON. */
void srv_mon_default_on(void);

#include "srv0mon.ic"
#else /* !UNIV_HOTBACKUP */
#define MONITOR_INC(x) ((void)0)
#define MONITOR_DEC(x) ((void)0)
#endif /* !UNIV_HOTBACKUP */



#endif
