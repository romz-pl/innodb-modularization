#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Deallocates the log recent closed buffer.
@param[out]	log	redo log */
void log_deallocate_recent_closed(log_t &log);
