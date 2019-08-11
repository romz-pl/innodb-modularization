#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Deallocates the log recent written buffer.
@param[out]	log	redo log */
void log_deallocate_recent_written(log_t &log);
