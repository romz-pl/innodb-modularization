#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Reads a log file header page to log.checkpoint_buf.
@param[in,out]	log	redo log
@param[in]	header	0 or LOG_CHECKPOINT_1 or LOG_CHECKPOINT2 */
void log_files_header_read(log_t &log, uint32_t header);

