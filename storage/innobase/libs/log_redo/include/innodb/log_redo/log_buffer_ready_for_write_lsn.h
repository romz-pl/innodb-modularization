#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/lsn_t.h>

struct log_t;

/** @return lsn up to which all writes to log buffer have been finished */
lsn_t log_buffer_ready_for_write_lsn(const log_t &log);

#endif
