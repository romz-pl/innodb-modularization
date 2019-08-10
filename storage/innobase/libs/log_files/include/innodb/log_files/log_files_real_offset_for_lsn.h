#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

struct log_t;

/** Calculates offset within log files, including headers of log files,
for the provided lsn value.
@param[in]	log	redo log
@param[in]	lsn	log sequence number
@return real offset within the log files */
uint64_t log_files_real_offset_for_lsn(const log_t &log, lsn_t lsn);
