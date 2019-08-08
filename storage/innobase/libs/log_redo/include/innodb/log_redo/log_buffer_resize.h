#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Changes size of the log buffer. This is a thread-safe version.
It is used by SET GLOBAL innodb_log_buffer_size = X.
@param[in,out]	log		redo log
@param[in]	new_size	requested new size
@return true iff succeeded in resize */
bool log_buffer_resize(log_t &log, size_t new_size);
