#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/lsn_t.h>

struct log_t;

/** Calculates age of current checkpoint as number of bytes since
last checkpoint. This includes bytes for headers and footers of
all log blocks. The calculation is based on the latest written
checkpoint lsn, and the current lsn, which points to the first
non reserved data byte. Note that the current lsn could not fit
the free space in the log files. This means that the checkpoint
age could potentially be larger than capacity of the log files.
However we do the best effort to avoid such situations, and if
they happen, user threads wait until the space is reclaimed.
@param[in]	log	redo log
@return checkpoint age as number of bytes */
lsn_t log_get_checkpoint_age(const log_t &log);

#endif
