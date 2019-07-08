#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/monitor/monitor_id_t.h>
#include <innodb/monitor/macros.h>

/** This "monitor_set_tbl" is a bitmap records whether a particular monitor
counter has been turned on or off */
extern ulint monitor_set_tbl[(NUM_MONITOR + NUM_BITS_ULINT - 1) / NUM_BITS_ULINT];

#endif
