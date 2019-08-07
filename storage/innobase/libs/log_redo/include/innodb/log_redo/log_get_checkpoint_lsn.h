#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/lsn_t.h>

struct log_t;

/** Gets the last checkpoint lsn stored and flushed to disk.
@return last checkpoint lsn */
lsn_t log_get_checkpoint_lsn(const log_t &log);

#endif
