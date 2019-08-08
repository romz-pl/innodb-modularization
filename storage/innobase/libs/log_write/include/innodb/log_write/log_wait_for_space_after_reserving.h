#pragma once

#include <innodb/univ/univ.h>

struct log_t;
struct Log_handle;


/** Waits until there is free space in log buffer up to reserved handle.end_sn.
If there was no space, it basically waits for log writer thread which copies
data from log buffer to log files and advances log.write_lsn, reclaiming space
in the log buffer (it's a ring buffer).

There is a special case - if it turned out, that log buffer is too small for
the reserved range of lsn values, it resizes the log buffer.

It's used during reservation of lsn values, when the reserved handle.end_sn is
greater than log.sn_limit_for_end.

@param[in,out]	log		redo log
@param[in]	handle		handle for the reservation */
void log_wait_for_space_after_reserving(log_t &log,
                                               const Log_handle &handle);
