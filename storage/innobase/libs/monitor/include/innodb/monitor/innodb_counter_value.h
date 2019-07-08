#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/monitor_value_t.h>
#include <innodb/monitor/monitor_info_t.h>

/** The actual monitor counter array that records each monintor counter
value */
extern monitor_value_t innodb_counter_value[NUM_MONITOR];

#endif
