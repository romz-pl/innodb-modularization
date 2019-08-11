#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

struct log_t;

/** Writes the next checkpoint info to header of the first log file.
Note that two pages of the header are used alternately for consecutive
checkpoints. If we crashed during the write, we would still have the
previous checkpoint info and recovery would work.
@param[in,out]	log			redo log
@param[in]	next_checkpoint_lsn	writes checkpoint at this lsn */
void log_files_write_checkpoint(log_t &log, lsn_t next_checkpoint_lsn);

