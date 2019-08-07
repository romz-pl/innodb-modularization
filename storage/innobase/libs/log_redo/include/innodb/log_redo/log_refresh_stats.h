#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Refreshes the statistics used to print per-second averages in log_print().
@param[in,out]	log	redo log */
void log_refresh_stats(log_t &log);
