#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Deallocates the array with write events.
@param[out]	log	redo log */
void log_deallocate_write_events(log_t &log);
