#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Advances log.buf_ready_for_write_lsn using links in the recent written
buffer. It's used by the log writer thread only.
@param[in]	log	redo log
@return true if and only if the lsn has been advanced */
bool log_advance_ready_for_write_lsn(log_t &log);
