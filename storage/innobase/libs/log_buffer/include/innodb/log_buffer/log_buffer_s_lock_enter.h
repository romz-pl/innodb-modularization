#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Acquires the log buffer s-lock.
@param[in,out]	log	redo log
@return lock no, must be passed to s_lock_exit() */
size_t log_buffer_s_lock_enter(log_t &log);
