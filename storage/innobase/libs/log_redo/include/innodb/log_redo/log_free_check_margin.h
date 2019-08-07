#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/sn_t.h>

struct log_t;

/** Calculates margin which has to be used in log_free_check() call,
when checking if user thread should wait for more space in redo log.
@return size of the margin to use */
sn_t log_free_check_margin(const log_t &log);
