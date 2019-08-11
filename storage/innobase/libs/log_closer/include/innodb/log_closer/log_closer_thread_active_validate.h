#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Validates that the log closer thread is active.
Used only to assert, that the state is correct.
@param[in]	log	redo log */
void log_closer_thread_active_validate(const log_t &log);
