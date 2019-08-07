#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Calculates offset within log files, excluding headers of log files.
@param[in]	log		redo log
@param[in]	offset		real offset (including log file headers)
@return	size offset excluding log file headers (<= offset) */
uint64_t log_files_size_offset(const log_t &log, uint64_t offset);
