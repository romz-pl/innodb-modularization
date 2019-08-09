#pragma once

#include <innodb/univ/univ.h>

struct log_t;

/** Executes a synchronous flush of the log files (doing fsyncs).
Advances log.flushed_to_disk_lsn and notifies log flush_notifier thread.
Note: if only a single log block was flushed to disk, user threads
waiting for lsns within the block are notified directly from here,
and log flush_notifier thread is not notified! (optimization)
@param[in,out]	log   redo log */
void log_flush_low(log_t &log);
