#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Deallocates the log write-ahead buffer.
@param[out]	log	redo log */
void log_deallocate_write_ahead_buffer(log_t &log);
