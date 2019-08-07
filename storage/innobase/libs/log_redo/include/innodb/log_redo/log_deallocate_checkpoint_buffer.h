#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Deallocates the log checkpoint buffer.
@param[out]	log	redo log */
void log_deallocate_checkpoint_buffer(log_t &log);
