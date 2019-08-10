#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Allocates the array with flush events.
@param[out]	log	redo log */
void log_allocate_flush_events(log_t &log);
