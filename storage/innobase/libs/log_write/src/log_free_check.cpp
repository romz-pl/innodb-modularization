#include <innodb/log_write/log_free_check.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/log_sys.h>
//#include <innodb/log_redo/log_free_check_wait.h>

void log_free_check_wait(log_t &log, sn_t sn);

/** Call this function before starting a mini-transaction.  It will check
for space in the redo log. It assures there is at least
concurrency_safe_free_margin.  If the space is not available, this will
wait until it is. Therefore it is important that the caller does not hold
any latch that may be called by the page cleaner or log flush process.
This includes any page block or file space latch. */
void log_free_check() {
  log_t &log = *log_sys;

#ifdef UNIV_DEBUG
  /* This function may be called while holding some latches. This is OK,
  as long as we are not holding any latches on buffer blocks or file spaces.
  The following latches are not held by any thread that frees up redo log
  space. */
  static const latch_level_t latches[] = {
      SYNC_NO_ORDER_CHECK, /* used for non-labeled latches */
      SYNC_RSEGS,          /* rsegs->x_lock in trx_rseg_create() */
      SYNC_UNDO_DDL,       /* undo::ddl_mutex */
      SYNC_UNDO_SPACES,    /* undo::spaces::m_latch */
      SYNC_FTS_CACHE,      /* fts_cache_t::lock */
      SYNC_DICT,           /* dict_sys->mutex in commit_try_rebuild() */
      SYNC_DICT_OPERATION, /* X-latch in commit_try_rebuild() */
      SYNC_INDEX_TREE      /* index->lock */
  };

  sync_allowed_latches check(latches,
                             latches + sizeof(latches) / sizeof(*latches));

  if (sync_check_iterate(check)) {
    ib::error()
        << "log_free_check() was called while holding an un-listed latch.";
    ut_error;
  }
#endif /* UNIV_DEBUG */

  /** We prefer to wait now for the space in log file, because now
  are not holding any latches of dirty pages. */

  const sn_t sn = log.sn.load();

  if (unlikely(sn > log.sn_limit_for_start.load())) {
    /* We need to wait, because the concurrency margin could be violated
    if we let all threads to go forward after making this check now.

    The waiting procedure is rather unlikely to happen for proper my.cnf.
    Therefore we extracted the code to seperate function, to make the
    inlined log_free_check() small. */

    log_free_check_wait(log, sn);
  }
}

#endif
