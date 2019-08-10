#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/Log_handle.h>
#include <innodb/log_sn/lsn_t.h>

struct log_t;

/** Writes data to the log buffer. The space in the redo log has to be
reserved before calling to this function and lsn pointing to inside the
reserved range of lsn values has to be provided.

The write does not have to cover the whole reserved space, but may not
overflow it. If it does not cover, then returned value should be used
to start the next write operation. Note that finally we must use exactly
all the reserved space.

@see @ref sect_redo_log_buf_write
@param[in,out]	log		redo log
@param[in]	handle		handle for the reservation of space
@param[in]	str		memory to write data from
@param[in]	str_len		number of bytes to write
@param[in]	start_lsn	lsn to start writing at (the reserved space)

@return end_lsn after writing the data (in the reserved space), could be
used to start the next write operation if there is still free space in
the reserved space */
lsn_t log_buffer_write(log_t &log, const Log_handle &handle, const byte *str,
                       size_t str_len, lsn_t start_lsn);


