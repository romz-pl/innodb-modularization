#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct log_t;

/** Write to the log file up to the last log entry.
@param[in,out]	log	redo log
@param[in]	sync	whether we want the written log
also to be flushed to disk. */
void log_buffer_flush_to_disk(log_t &log, bool sync = true);

/** Requests flush of the log buffer.
@param[in]	sync	true: wait until the flush is done */
void log_buffer_flush_to_disk(bool sync = true);

#endif
