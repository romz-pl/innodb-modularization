#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>
#include <innodb/wait/Wait_stats.h>

struct log_t;

Wait_stats log_wait_for_write(const log_t &log, lsn_t lsn);
