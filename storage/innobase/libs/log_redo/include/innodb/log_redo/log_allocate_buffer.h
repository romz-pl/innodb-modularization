#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Calculates proper size for the log buffer and allocates the log buffer.
@param[out]	log	redo log */
void log_allocate_buffer(log_t &log);
