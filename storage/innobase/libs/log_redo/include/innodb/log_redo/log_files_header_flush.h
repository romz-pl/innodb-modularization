#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

struct log_t;

/** Writes a log file header to the log file space.
@param[in]	log		redo log
@param[in]	nth_file	header for the nth file in the log files
@param[in]	start_lsn	log file data starts at this lsn */
void log_files_header_flush(log_t &log, uint32_t nth_file, lsn_t start_lsn);
