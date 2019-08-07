#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Calculates proper size of the log buffer and updates related fields.
Calculations are based on current value of srv_log_buffer_size. Note,
that the proper size of the log buffer should be a power of two.
@param[out]	log		redo log */
void log_calc_buf_size(log_t &log);
