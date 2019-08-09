#include <innodb/log_redo/log_calc_max_ages.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_redo/log_calc_concurrency_margin.h>
#include <innodb/log_types/log_t.h>

bool log_calc_max_ages(log_t &log) {
  ut_ad(log_writer_mutex_own(log));

  log.lsn_real_capacity =
      log.files_real_capacity - LOG_FILE_HDR_SIZE * log.n_files;

  /* Add safety margin, disallowed to be used (never, ever). */
  const lsn_t safety_margin =
      std::min(static_cast<lsn_t>(0.1 * log.lsn_real_capacity),
               static_cast<lsn_t>(256 * LOG_CHECKPOINT_FREE_PER_THREAD *
                                  UNIV_PAGE_SIZE));

  ut_a(log.lsn_real_capacity > safety_margin + OS_FILE_LOG_BLOCK_SIZE * 8);

  log.lsn_capacity_for_writer = ut_uint64_align_down(
      log.lsn_real_capacity - safety_margin, OS_FILE_LOG_BLOCK_SIZE);

  /* Extra margin used for emergency increase of the concurrency_margin. */
  log.extra_margin = ut_uint64_align_down(
      static_cast<lsn_t>(log.lsn_capacity_for_writer * 0.05),
      OS_FILE_LOG_BLOCK_SIZE);

  /* Users stop in log-free-check call before they enter the extra_margin,
  the log_writer can still go forward through the extra_margin, triggering
  the emergency increase of concurrency_margin mean while. */
  log.lsn_capacity_for_free_check =
      log.lsn_capacity_for_writer - log.extra_margin;

  log.max_concurrency_margin = ut_uint64_align_up(
      log.lsn_capacity_for_writer / 2, OS_FILE_LOG_BLOCK_SIZE);

  const bool success = log_calc_concurrency_margin(log);

  log.concurrency_margin_ok = true;

  /* Set limits used in flushing and checkpointing mechanism. */

  const lsn_t limit = log.lsn_capacity_for_free_check;

  log.max_modified_age_sync = limit - limit / LOG_POOL_PREFLUSH_RATIO_SYNC;

  log.max_modified_age_async = limit - limit / LOG_POOL_PREFLUSH_RATIO_ASYNC;

  log.max_checkpoint_age_async =
      limit - limit / LOG_POOL_CHECKPOINT_RATIO_ASYNC;

  return success;
}
