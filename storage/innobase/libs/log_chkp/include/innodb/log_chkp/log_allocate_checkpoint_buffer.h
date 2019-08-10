#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Allocates the log checkpoint buffer (used to write checkpoint headers).
@param[out]	log	redo log */
void log_allocate_checkpoint_buffer(log_t &log);
