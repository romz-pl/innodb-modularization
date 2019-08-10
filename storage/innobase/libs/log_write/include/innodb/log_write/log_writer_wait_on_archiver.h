#pragma once

#include <innodb/univ/univ.h>

struct log_t;

#include <innodb/log_sn/lsn_t.h>

/* Waits until the archiver has archived enough for log_writer to proceed
or until the archiver becomes aborted.
@param[in]  log             redo log
@param[in]  last_write_lsn  previous log.write_lsn
@param[in]  next_write_lsn  next log.write_lsn */
void log_writer_wait_on_archiver(log_t &log, lsn_t last_write_lsn,
                                        lsn_t next_write_lsn);
