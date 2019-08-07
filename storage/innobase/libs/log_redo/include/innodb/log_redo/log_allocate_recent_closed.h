#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Allocates the log recent closed buffer.
@param[out]	log	redo log */
void log_allocate_recent_closed(log_t &log);
