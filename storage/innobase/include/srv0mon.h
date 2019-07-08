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




#ifndef UNIV_HOTBACKUP


/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */
#define MONITOR_FIELD(monitor, field) (innodb_counter_value[monitor].field)

#define MONITOR_VALUE(monitor) MONITOR_FIELD(monitor, mon_value)

#define MONITOR_MAX_VALUE(monitor) MONITOR_FIELD(monitor, mon_max_value)

#define MONITOR_MIN_VALUE(monitor) MONITOR_FIELD(monitor, mon_min_value)

#define MONITOR_VALUE_RESET(monitor) MONITOR_FIELD(monitor, mon_value_reset)

#define MONITOR_MAX_VALUE_START(monitor) \
  MONITOR_FIELD(monitor, mon_max_value_start)

#define MONITOR_MIN_VALUE_START(monitor) \
  MONITOR_FIELD(monitor, mon_min_value_start)

#define MONITOR_LAST_VALUE(monitor) MONITOR_FIELD(monitor, mon_last_value)

#define MONITOR_START_VALUE(monitor) MONITOR_FIELD(monitor, mon_start_value)

#define MONITOR_VALUE_SINCE_START(monitor) \
  (MONITOR_VALUE(monitor) + MONITOR_VALUE_RESET(monitor))

#define MONITOR_STATUS(monitor) MONITOR_FIELD(monitor, mon_status)

#define MONITOR_SET_START(monitor)                         \
  do {                                                     \
    MONITOR_STATUS(monitor) = MONITOR_STARTED;             \
    MONITOR_FIELD((monitor), mon_start_time) = time(NULL); \
  } while (0)

#define MONITOR_SET_OFF(monitor)                          \
  do {                                                    \
    MONITOR_STATUS(monitor) = MONITOR_STOPPED;            \
    MONITOR_FIELD((monitor), mon_stop_time) = time(NULL); \
  } while (0)

#define MONITOR_INIT_ZERO_VALUE 0

/** Max and min values are initialized when we first turn on the monitor
counter, and set the MONITOR_STATUS. */
#define MONITOR_MAX_MIN_NOT_INIT(monitor)                   \
  (MONITOR_STATUS(monitor) == MONITOR_INIT_ZERO_VALUE &&    \
   MONITOR_MIN_VALUE(monitor) == MONITOR_INIT_ZERO_VALUE && \
   MONITOR_MAX_VALUE(monitor) == MONITOR_INIT_ZERO_VALUE)

#define MONITOR_INIT(monitor)                        \
  if (MONITOR_MAX_MIN_NOT_INIT(monitor)) {           \
    MONITOR_MIN_VALUE(monitor) = MIN_RESERVED;       \
    MONITOR_MIN_VALUE_START(monitor) = MIN_RESERVED; \
    MONITOR_MAX_VALUE(monitor) = MAX_RESERVED;       \
    MONITOR_MAX_VALUE_START(monitor) = MAX_RESERVED; \
  }

/** Macros to increment/decrement the counters. The normal
monitor counter operation expects appropriate synchronization
already exists. No additional mutex is necessary when operating
on the counters */
#define MONITOR_INC(monitor)                                   \
  if (MONITOR_IS_ON(monitor)) {                                \
    MONITOR_VALUE(monitor)++;                                  \
    if (MONITOR_VALUE(monitor) > MONITOR_MAX_VALUE(monitor)) { \
      MONITOR_MAX_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
  }

/** Atomically increment a monitor counter.
Use MONITOR_INC if appropriate mutex protection exists.
@param monitor monitor to be incremented by 1 */
#define MONITOR_ATOMIC_INC(monitor)                                            \
  if (MONITOR_IS_ON(monitor)) {                                                \
    ib_uint64_t value;                                                         \
    value =                                                                    \
        os_atomic_increment_uint64((ib_uint64_t *)&MONITOR_VALUE(monitor), 1); \
    /* Note: This is not 100% accurate because of the                          \
    inherent race, we ignore it due to performance. */                         \
    if (value > (ib_uint64_t)MONITOR_MAX_VALUE(monitor)) {                     \
      MONITOR_MAX_VALUE(monitor) = value;                                      \
    }                                                                          \
  }

/** Atomically decrement a monitor counter.
Use MONITOR_DEC if appropriate mutex protection exists.
@param monitor monitor to be decremented by 1 */
#define MONITOR_ATOMIC_DEC(monitor)                                            \
  if (MONITOR_IS_ON(monitor)) {                                                \
    ib_uint64_t value;                                                         \
    value =                                                                    \
        os_atomic_decrement_uint64((ib_uint64_t *)&MONITOR_VALUE(monitor), 1); \
    /* Note: This is not 100% accurate because of the                          \
    inherent race, we ignore it due to performance. */                         \
    if (value < (ib_uint64_t)MONITOR_MIN_VALUE(monitor)) {                     \
      MONITOR_MIN_VALUE(monitor) = value;                                      \
    }                                                                          \
  }

#define MONITOR_DEC(monitor)                                   \
  if (MONITOR_IS_ON(monitor)) {                                \
    MONITOR_VALUE(monitor)--;                                  \
    if (MONITOR_VALUE(monitor) < MONITOR_MIN_VALUE(monitor)) { \
      MONITOR_MIN_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
  }

#ifdef UNIV_DEBUG_VALGRIND
#define MONITOR_CHECK_DEFINED(value)  \
  do {                                \
    mon_type_t m = value;             \
    UNIV_MEM_ASSERT_RW(&m, sizeof m); \
  } while (0)
#else /* UNIV_DEBUG_VALGRIND */
#define MONITOR_CHECK_DEFINED(value) (void)0
#endif /* UNIV_DEBUG_VALGRIND */

#define MONITOR_INC_VALUE(monitor, value)                      \
  MONITOR_CHECK_DEFINED(value);                                \
  if (MONITOR_IS_ON(monitor)) {                                \
    MONITOR_VALUE(monitor) += (mon_type_t)(value);             \
    if (MONITOR_VALUE(monitor) > MONITOR_MAX_VALUE(monitor)) { \
      MONITOR_MAX_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
  }

#define MONITOR_DEC_VALUE(monitor, value)                                     \
  MONITOR_CHECK_DEFINED(value);                                               \
  if (MONITOR_IS_ON(monitor)) {                                               \
                ut_ad(MONITOR_VALUE(monitor) >= (mon_type_t) (value);	\
        MONITOR_VALUE(monitor) -= (mon_type_t) (value);		\
        if (MONITOR_VALUE(monitor) < MONITOR_MIN_VALUE(monitor)) {  \
            MONITOR_MIN_VALUE(monitor) = MONITOR_VALUE(monitor);\
        }                                                             \
  }

/* Increment/decrement counter without check the monitor on/off bit, which
could already be checked as a module group */
#define MONITOR_INC_NOCHECK(monitor)                           \
  do {                                                         \
    MONITOR_VALUE(monitor)++;                                  \
    if (MONITOR_VALUE(monitor) > MONITOR_MAX_VALUE(monitor)) { \
      MONITOR_MAX_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
  } while (0)

#define MONITOR_DEC_NOCHECK(monitor)                           \
  do {                                                         \
    MONITOR_VALUE(monitor)--;                                  \
    if (MONITOR_VALUE(monitor) < MONITOR_MIN_VALUE(monitor)) { \
      MONITOR_MIN_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
  } while (0)

/** Directly set a monitor counter's value */
#define MONITOR_SET(monitor, value)                            \
  MONITOR_CHECK_DEFINED(value);                                \
  if (MONITOR_IS_ON(monitor)) {                                \
    MONITOR_VALUE(monitor) = (mon_type_t)(value);              \
    if (MONITOR_VALUE(monitor) > MONITOR_MAX_VALUE(monitor)) { \
      MONITOR_MAX_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
    if (MONITOR_VALUE(monitor) < MONITOR_MIN_VALUE(monitor)) { \
      MONITOR_MIN_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
  }

/** Add time difference between now and input "value" (in seconds) to the
monitor counter
@param monitor monitor to update for the time difference
@param value the start time value */
#define MONITOR_INC_TIME_IN_MICRO_SECS(monitor, value)        \
  MONITOR_CHECK_DEFINED(value);                               \
  if (MONITOR_IS_ON(monitor)) {                               \
    uintmax_t old_time = (value);                             \
    value = ut_time_us(NULL);                                 \
    MONITOR_VALUE(monitor) += (mon_type_t)(value - old_time); \
  }

/** This macro updates 3 counters in one call. However, it only checks the
main/first monitor counter 'monitor', to see it is on or off to decide
whether to do the update.
@param monitor the main monitor counter to update. It accounts for
                        the accumulative value for the counter.
@param monitor_n_calls counter that counts number of times this macro is
                        called
@param monitor_per_call counter that records the current and max value of
                        each incremental value
@param value incremental value to record this time */
#define MONITOR_INC_VALUE_CUMULATIVE(monitor, monitor_n_calls,   \
                                     monitor_per_call, value)    \
  MONITOR_CHECK_DEFINED(value);                                  \
  if (MONITOR_IS_ON(monitor)) {                                  \
    MONITOR_VALUE(monitor_n_calls)++;                            \
    MONITOR_VALUE(monitor_per_call) = (mon_type_t)(value);       \
    if (MONITOR_VALUE(monitor_per_call) >                        \
        MONITOR_MAX_VALUE(monitor_per_call)) {                   \
      MONITOR_MAX_VALUE(monitor_per_call) = (mon_type_t)(value); \
    }                                                            \
    MONITOR_VALUE(monitor) += (mon_type_t)(value);               \
    if (MONITOR_VALUE(monitor) > MONITOR_MAX_VALUE(monitor)) {   \
      MONITOR_MAX_VALUE(monitor) = MONITOR_VALUE(monitor);       \
    }                                                            \
  }

/** Directly set a monitor counter's value, and if the value
is monotonically increasing, only max value needs to be updated */
#define MONITOR_SET_UPD_MAX_ONLY(monitor, value)               \
  MONITOR_CHECK_DEFINED(value);                                \
  if (MONITOR_IS_ON(monitor)) {                                \
    MONITOR_VALUE(monitor) = (mon_type_t)(value);              \
    if (MONITOR_VALUE(monitor) > MONITOR_MAX_VALUE(monitor)) { \
      MONITOR_MAX_VALUE(monitor) = MONITOR_VALUE(monitor);     \
    }                                                          \
  }

/** Some values such as log sequence number are montomically increasing
number, do not need to record max/min values */
#define MONITOR_SET_SIMPLE(monitor, value)        \
  MONITOR_CHECK_DEFINED(value);                   \
  if (MONITOR_IS_ON(monitor)) {                   \
    MONITOR_VALUE(monitor) = (mon_type_t)(value); \
  }

/** Reset the monitor value and max/min value to zero. The reset
operation would only be conducted when the counter is turned off */
#define MONITOR_RESET_ALL(monitor)                                    \
  do {                                                                \
    MONITOR_VALUE(monitor) = MONITOR_INIT_ZERO_VALUE;                 \
    MONITOR_MAX_VALUE(monitor) = MAX_RESERVED;                        \
    MONITOR_MIN_VALUE(monitor) = MIN_RESERVED;                        \
    MONITOR_VALUE_RESET(monitor) = MONITOR_INIT_ZERO_VALUE;           \
    MONITOR_MAX_VALUE_START(monitor) = MAX_RESERVED;                  \
    MONITOR_MIN_VALUE_START(monitor) = MIN_RESERVED;                  \
    MONITOR_LAST_VALUE(monitor) = MONITOR_INIT_ZERO_VALUE;            \
    MONITOR_FIELD(monitor, mon_start_time) = MONITOR_INIT_ZERO_VALUE; \
    MONITOR_FIELD(monitor, mon_stop_time) = MONITOR_INIT_ZERO_VALUE;  \
    MONITOR_FIELD(monitor, mon_reset_time) = MONITOR_INIT_ZERO_VALUE; \
  } while (0)

/** Following four macros defines necessary operations to fetch and
consolidate information from existing system status variables. */

/** Save the passed-in value to mon_start_value field of monitor
counters */
#define MONITOR_SAVE_START(monitor, value)                  \
  do {                                                      \
    MONITOR_CHECK_DEFINED(value);                           \
    (MONITOR_START_VALUE(monitor) =                         \
         (mon_type_t)(value)-MONITOR_VALUE_RESET(monitor)); \
  } while (0)

/** Save the passed-in value to mon_last_value field of monitor
counters */
#define MONITOR_SAVE_LAST(monitor)                          \
  do {                                                      \
    MONITOR_LAST_VALUE(monitor) = MONITOR_VALUE(monitor);   \
    MONITOR_START_VALUE(monitor) += MONITOR_VALUE(monitor); \
  } while (0)

/** Set monitor value to the difference of value and mon_start_value
compensated by mon_last_value if accumulated value is required. */
#define MONITOR_SET_DIFF(monitor, value)                                       \
  MONITOR_SET_UPD_MAX_ONLY(monitor, ((value)-MONITOR_VALUE_RESET(monitor) -    \
                                     MONITOR_FIELD(monitor, mon_start_value) + \
                                     MONITOR_FIELD(monitor, mon_last_value)))

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

#define MONITOR_INC_WAIT_STATS_EX(monitor_prefix, monitor_sufix, wait_stats) \
  if ((wait_stats).wait_loops == 0) {                                        \
    MONITOR_INC(monitor_prefix##NO_WAITS##monitor_sufix);                    \
  } else {                                                                   \
    MONITOR_INC(monitor_prefix##WAITS##monitor_sufix);                       \
    MONITOR_INC_VALUE(monitor_prefix##WAIT_LOOPS##monitor_sufix,             \
                      (wait_stats).wait_loops);                              \
  }

#define MONITOR_INC_WAIT_STATS(monitor_prefix, wait_stats) \
  MONITOR_INC_WAIT_STATS_EX(monitor_prefix, , wait_stats);

#endif
