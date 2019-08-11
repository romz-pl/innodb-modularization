#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Unlock the locked redo log.
@param[in,out]	log	redo log to unlock */
void log_position_unlock(log_t &log);
