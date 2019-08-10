#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

/** Fill redo log header.
@param[out]	buf		filled buffer
@param[in]	start_lsn	log start LSN
@param[in]	creator		creator of the header */
void log_files_header_fill(byte *buf, lsn_t start_lsn, const char *creator);
