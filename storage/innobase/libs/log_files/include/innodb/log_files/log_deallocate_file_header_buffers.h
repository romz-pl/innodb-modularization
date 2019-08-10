#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Deallocates buffers for headers of the log files.
@param[out]	log	redo log */
void log_deallocate_file_header_buffers(log_t &log);
