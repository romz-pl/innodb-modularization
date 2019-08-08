#pragma once

#include <innodb/univ/univ.h>

struct log_t;
struct Log_handle;

/** Adds a link start_lsn -> end_lsn to the log recent closed buffer.

This is called after all dirty pages related to [start_lsn, end_lsn)
have been added to corresponding flush lists.
For detailed explanation - @see log0write.cc.

@see @ref sect_redo_log_add_link_to_recent_closed
@param[in,out]	log		redo log
@param[in]	handle		handle for the reservation of space */
void log_buffer_close(log_t &log, const Log_handle &handle);


