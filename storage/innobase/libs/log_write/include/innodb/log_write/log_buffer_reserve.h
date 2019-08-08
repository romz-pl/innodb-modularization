#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/Log_handle.h>

struct log_t;

/** Reserves space in the redo log for following write operations.
Space is reserved for a given number of data bytes. Additionally
bytes for required headers and footers of log blocks are reserved.

After the space is reserved, range of lsn values from a start_lsn
to an end_lsn is assigned. The log writer thread cannot proceed
further than to the start_lsn, until a link start_lsn -> end_lsn
has been added to the log recent written buffer.

NOTE that the link is added after data is written to the reserved
space in the log buffer. It is very critical to do all these steps
as fast as possible, because very likely the log writer thread is
waiting for the link.

@see @ref sect_redo_log_buf_reserve
@param[in,out]	log	redo log
@param[in]	len	number of data bytes to reserve for write
@return handle that represents the reservation */
Log_handle log_buffer_reserve(log_t &log, size_t len);
