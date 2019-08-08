#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/lsn_t.h>

struct log_t;
struct Log_handle;

/** Modifies header of log block in the log buffer, which contains
a given lsn value, and sets offset to the first group of log records
within the block.

This is used by mtr after writing a log record group which ends at
lsn belonging to different log block than lsn at which the group was
started. When write was finished at the last data byte of log block,
it is considered ended in the next log block, because the next data
byte belongs to that block.

During recovery, when recovery is started in the middle of some group
of log records, it first looks for the beginning of the next group.

@param[in,out]	log			redo log
@param[in]	handle			handle for the reservation of space
@param[in]	rec_group_end_lsn	lsn at which the first log record
group starts within the block containing this lsn value */
void log_buffer_set_first_record_group(log_t &log, const Log_handle &handle,
                                       lsn_t rec_group_end_lsn);
