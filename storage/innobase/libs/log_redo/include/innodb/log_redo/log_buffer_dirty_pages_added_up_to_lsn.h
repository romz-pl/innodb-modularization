#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_sn/lsn_t.h>

struct log_t;

/** @return lsn up to which all dirty pages have been added to flush list */
lsn_t log_buffer_dirty_pages_added_up_to_lsn(const log_t &log);

#endif
