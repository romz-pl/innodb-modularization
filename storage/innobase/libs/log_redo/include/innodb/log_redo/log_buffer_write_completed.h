#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

struct log_t;
struct Log_handle;

/** Adds a link start_lsn -> end_lsn to the log recent written buffer.

This function must be called after the data has been written to the
fragment of log buffer represented by range [start_lsn, end_lsn).
After the link is added, the log writer may write the data to disk.

NOTE that still dirty pages for the [start_lsn, end_lsn) are not added
to flush lists when this function is called.

@see @ref sect_redo_log_buf_add_links_to_recent_written
@param[in,out]	log		redo log
@param[in]	handle		handle for the reservation of space
@param[in]	start_lsn	start_lsn of the link to add
@param[in]	end_lsn		end_lsn of the link to add */
void log_buffer_write_completed(log_t &log, const Log_handle &handle,
                                lsn_t start_lsn, lsn_t end_lsn);

