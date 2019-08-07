#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Releases the log buffer s-lock.
@param[in,out]	log	redo log
@param[in]	lock_no	lock no received from s_lock_enter() */
void log_buffer_s_lock_exit(log_t &log, size_t lock_no);
