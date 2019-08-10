#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Acquires the log buffer x-lock.
@param[in,out]	log	redo log */
void log_buffer_x_lock_enter(log_t &log);
