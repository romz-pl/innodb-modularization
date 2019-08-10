#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

struct log_t;

/** Changes size of the log buffer. This is a non-thread-safe version
which might be invoked only when there are no concurrent possible writes
to the log buffer. It is used in log_buffer_reserve() when a requested
size to reserve is larger than size of the log buffer.
@param[in,out]	log		redo log
@param[in]	new_size	requested new size
@param[in]	end_lsn		maximum lsn written to log buffer
@return true iff succeeded in resize */
bool log_buffer_resize_low(log_t &log, size_t new_size, lsn_t end_lsn);
