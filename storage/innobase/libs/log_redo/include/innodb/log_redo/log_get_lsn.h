#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct log_t;

#include <innodb/log_types/lsn_t.h>

/** Gets the current lsn value. This value points to the first non
reserved data byte in the redo log. When next user thread reserves
space in the redo log, it starts at this lsn.

If the last reservation finished exactly before footer of log block,
this value points to the first byte after header of the next block.

NOTE that it is possible that the current lsn value does not fit
free space in the log files or in the log buffer. In such case,
user threads need to wait until the space becomes available.

@return current lsn */
lsn_t log_get_lsn(const log_t &log);

#endif
