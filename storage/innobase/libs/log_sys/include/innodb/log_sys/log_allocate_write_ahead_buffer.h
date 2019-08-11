#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Allocates the log write-ahead buffer (aligned to system page for
easier migrations between NUMA nodes).
@param[out]	log	redo log */
void log_allocate_write_ahead_buffer(log_t &log);
