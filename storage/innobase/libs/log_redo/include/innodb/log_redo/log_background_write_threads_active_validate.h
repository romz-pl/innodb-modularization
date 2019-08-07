#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Validates that the log writer, flusher threads are active.
Used only to assert, that the state is correct.
@param[in]	log	redo log */
void log_background_write_threads_active_validate(const log_t &log);
