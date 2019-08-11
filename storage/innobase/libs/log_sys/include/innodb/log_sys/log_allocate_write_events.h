#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Allocates the array with write events.
@param[out]	log	redo log */
void log_allocate_write_events(log_t &log);
