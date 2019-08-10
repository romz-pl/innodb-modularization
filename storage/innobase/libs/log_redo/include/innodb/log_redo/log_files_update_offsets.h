#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_sn/lsn_t.h>

struct log_t;

/** Updates current_file_lsn and current_file_real_offset to correspond
to a given lsn. For this function to work, the values must already be
initialized to correspond to some lsn, for instance, a checkpoint lsn.
@param[in,out]	log	redo log
@param[in]	lsn	log sequence number to set files_start_lsn at */
void log_files_update_offsets(log_t &log, lsn_t lsn);

#endif
