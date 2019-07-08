#include <innodb/monitor/monitor_set_tbl.h>

#ifndef UNIV_HOTBACKUP

/* monitor_set_tbl is used to record and determine whether a monitor
has been turned on/off. */
ulint monitor_set_tbl[(NUM_MONITOR + NUM_BITS_ULINT - 1) / NUM_BITS_ULINT];

#endif

