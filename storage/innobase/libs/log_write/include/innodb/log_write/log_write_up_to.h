#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>
#include <innodb/wait/Wait_stats.h>

struct log_t;

/** Waits until the redo log is written up to a provided lsn.
@param[in]  log             redo log
@param[in]  lsn             lsn to wait for
@param[in]  flush_to_disk   true: wait until it is flushed
@return statistics about waiting inside */
Wait_stats log_write_up_to(log_t &log, lsn_t lsn, bool flush_to_disk);
