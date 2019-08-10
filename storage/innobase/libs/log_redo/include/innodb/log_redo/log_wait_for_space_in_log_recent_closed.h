#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>

struct log_t;

/** Waits until there is free space in the log recent closed buffer
for any links start_lsn -> end_lsn, which start at provided start_lsn.
It does not add any link.

This is called just before dirty pages for [start_lsn, end_lsn)
are added to flush lists. That's because we need to guarantee,
that the delay until dirty page is added to flush list is limited.
For detailed explanation - @see log0write.cc.

@see @ref sect_redo_log_add_dirty_pages
@param[in,out]	log   redo log
@param[in]      lsn   lsn on which we wait (for any link: lsn -> x) */
void log_wait_for_space_in_log_recent_closed(log_t &log, lsn_t lsn);
