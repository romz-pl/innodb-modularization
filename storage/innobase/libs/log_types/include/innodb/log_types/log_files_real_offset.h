#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Calculates offset within log files, including headers of log files.
@param[in]	log		redo log
@param[in]	offset		size offset (excluding log file headers)
@return real offset including log file headers (>= offset) */
uint64_t log_files_real_offset(const log_t &log, uint64_t offset);
