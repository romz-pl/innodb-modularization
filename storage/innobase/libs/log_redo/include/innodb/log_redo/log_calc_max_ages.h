#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Calculates limits for maximum age of checkpoint and maximum age of
the oldest page. Uses current value of srv_thread_concurrency.
@param[in,out]	log	redo log
@retval true if success
@retval false if the redo log is too small to accommodate the number of
OS threads in the database server */
bool log_calc_max_ages(log_t &log);
