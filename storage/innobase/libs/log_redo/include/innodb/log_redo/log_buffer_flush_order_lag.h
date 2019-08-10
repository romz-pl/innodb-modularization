#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_sn/lsn_t.h>

struct log_t;

/** @return capacity of the recent_closed, or 0 if !log_use_threads() */
lsn_t log_buffer_flush_order_lag(const log_t &log);

#endif
