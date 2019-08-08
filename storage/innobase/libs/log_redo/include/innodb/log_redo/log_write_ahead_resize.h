#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Resizes the write ahead buffer in the redo log.
@param[in,out]	log		redo log
@param[in]	new_size	new size (in bytes) */
void log_write_ahead_resize(log_t &log, size_t new_size);
