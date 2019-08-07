#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Validates that all the log background threads are inactive.
Used only to assert, that the state is correct.
@param[in]	log	redo log */
void log_background_threads_inactive_validate(const log_t &log);
