#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Advances log.buf_dirty_pages_added_up_to_lsn using links in the recent
closed buffer. It's used by the log closer thread only.
@param[in]	log	redo log
@return true if and only if the lsn has been advanced */
bool log_advance_dirty_pages_added_up_to_lsn(log_t &log);
