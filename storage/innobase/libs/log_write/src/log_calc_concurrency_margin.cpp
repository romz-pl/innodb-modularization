#include <innodb/log_write/log_calc_concurrency_margin.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_types/log_writer_mutex_own.h>
#include <innodb/log_write/srv_thread_concurrency.h>
#include <innodb/log_types/log_t.h>
#include <innodb/monitor/MONITOR_SET.h>

bool log_calc_concurrency_margin(log_t &log) {
  ut_ad(log_writer_mutex_own(log));

  uint64_t concurrency_margin;

  /* Single thread, which keeps latches of dirty pages, that block
  the checkpoint to advance, will have to finish writes to redo.
  It won't write more than LOG_CHECKPOINT_FREE_PER_THREAD, before
  it checks, if it should wait for the checkpoint (log_free_check()). */
  concurrency_margin = LOG_CHECKPOINT_FREE_PER_THREAD * UNIV_PAGE_SIZE;

  /* We will have at most that many threads, that need to release
  the latches. Note, that each thread will notice, that checkpoint
  is required and will wait until it's done in log_free_check(). */
  concurrency_margin *= 10 + srv_thread_concurrency;

  /* Add constant extra safety */
  concurrency_margin += LOG_CHECKPOINT_EXTRA_FREE * UNIV_PAGE_SIZE;

  /* Add extra safety calculated from redo-size. It is 5% of 90%
  of the real capacity (lsn_capacity is 90% of the real capacity). */
  concurrency_margin += ut_uint64_align_down(
      static_cast<uint64_t>(0.05 * log.lsn_capacity_for_writer),
      OS_FILE_LOG_BLOCK_SIZE);

  bool success;
  if (concurrency_margin > log.max_concurrency_margin) {
    concurrency_margin = log.max_concurrency_margin;
    success = false;
  } else {
    success = true;
  }

  MONITOR_SET(MONITOR_LOG_CONCURRENCY_MARGIN, concurrency_margin);
  log.concurrency_margin.store(concurrency_margin);
  return success;
}
