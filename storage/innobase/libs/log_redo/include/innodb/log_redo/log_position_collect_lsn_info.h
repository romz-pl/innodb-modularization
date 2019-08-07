#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

struct log_t;

/** Collect coordinates in the locked redo log.
@param[in]	log		locked redo log
@param[out]	current_lsn	stores current lsn there
@param[out]	checkpoint_lsn	stores checkpoint lsn there */
void log_position_collect_lsn_info(const log_t &log, lsn_t *current_lsn,
                                   lsn_t *checkpoint_lsn);
