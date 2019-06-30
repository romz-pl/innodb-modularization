#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_stats_t.h>

#ifndef UNIV_HOTBACKUP
/** Counters for RW locks. */
extern rw_lock_stats_t rw_lock_stats;
#endif /* !UNIV_HOTBACKUP */
