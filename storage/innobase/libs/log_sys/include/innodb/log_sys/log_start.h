#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>
#include <innodb/log_types/checkpoint_no_t.h>

struct log_t;

/** Starts the initialized redo log system using a provided
checkpoint_lsn and current lsn.
@param[in,out]	log		redo log
@param[in]	checkpoint_no	checkpoint no (sequential number)
@param[in]	checkpoint_lsn	checkpoint lsn
@param[in]	start_lsn	current lsn to start at */
void log_start(log_t &log, checkpoint_no_t checkpoint_no, lsn_t checkpoint_lsn,
               lsn_t start_lsn);
