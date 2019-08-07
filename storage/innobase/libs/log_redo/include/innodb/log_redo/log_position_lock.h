#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Lock redo log. Both current lsn and checkpoint lsn will not change
until the redo log is unlocked.
@param[in,out]	log	redo log to lock */
void log_position_lock(log_t &log);
