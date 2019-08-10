#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Deallocates the array with flush events.
@param[out]	log	redo log */
void log_deallocate_flush_events(log_t &log);
