#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Allocates the log recent written buffer.
@param[out]	log	redo log */
void log_allocate_recent_written(log_t &log);
