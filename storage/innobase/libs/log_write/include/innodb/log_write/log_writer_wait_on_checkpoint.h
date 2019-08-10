#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

struct log_t;

/* Waits until there is free space in log files for log_writer to proceed.
@param[in]  log             redo log
@param[in]  last_write_lsn  previous log.write_lsn
@param[in]  next_write_lsn  next log.write_lsn
@return lsn up to which possible write is limited */
lsn_t log_writer_wait_on_checkpoint(log_t &log, lsn_t last_write_lsn,
                                           lsn_t next_write_lsn);
