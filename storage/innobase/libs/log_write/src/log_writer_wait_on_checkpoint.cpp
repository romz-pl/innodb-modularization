#include <innodb/log_write/log_writer_wait_on_checkpoint.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_redo/log_advance_ready_for_write_lsn.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/log_increase_concurrency_margin.h>
#include <innodb/monitor/MONITOR_INC.h>

void log_request_checkpoint(log_t &log, bool sync);

lsn_t log_writer_wait_on_checkpoint(log_t &log, lsn_t last_write_lsn,
                                           lsn_t next_write_lsn) {
  const int32_t SLEEP_BETWEEN_RETRIES_IN_US = 100; /* 100us */

  const int32_t TIME_UNTIL_ERROR_IN_US = 5000000; /* 5s */

  ut_ad(log_writer_mutex_own(log));

  int32_t count = 1;
  lsn_t checkpoint_limited_lsn = LSN_MAX;

  while (true) {
    lsn_t checkpoint_lsn = log.last_checkpoint_lsn.load();

    checkpoint_lsn =
        ut_uint64_align_down(checkpoint_lsn, OS_FILE_LOG_BLOCK_SIZE);

    checkpoint_limited_lsn = checkpoint_lsn + log.lsn_capacity_for_writer;

    ut_a(last_write_lsn <= checkpoint_limited_lsn);
    ut_a(next_write_lsn > checkpoint_lsn);

    if (next_write_lsn + log.extra_margin <= checkpoint_limited_lsn) {
      log.concurrency_margin_ok = true;
      break;
    }

    if (log.concurrency_margin_ok) {
      log.concurrency_margin_ok = false;
      log_increase_concurrency_margin(log);
    }

    if (last_write_lsn + OS_FILE_LOG_BLOCK_SIZE <= checkpoint_limited_lsn) {
      /* Write what we have - adjust the speed to speed of checkpoints
      going forward (to speed of page-cleaners). */
      break;
    }

    (void)log_advance_ready_for_write_lsn(log);

    const int32_t ATTEMPTS_UNTIL_ERROR =
        TIME_UNTIL_ERROR_IN_US / SLEEP_BETWEEN_RETRIES_IN_US;

    if (count % ATTEMPTS_UNTIL_ERROR == 0) {
      /* We could not reclaim even single redo block for 5sec */

      ib::error(ER_IB_MSG_1234) << "Out of space in the redo log."
                                   " Checkpoint LSN: "
                                << checkpoint_lsn << ".";
    }

    log_writer_mutex_exit(log);

    /* We don't want to ask for sync checkpoint, because it
    is possible, that the oldest dirty page is latched and
    user thread, which keeps the latch, is waiting for space
    in log buffer (for log_writer writing to disk). In such
    case it would be deadlock (we can't flush the latched
    page and advance the checkpoint). We only ask for the
    checkpoint, and wait for some time. */
    log_request_checkpoint(log, false);

    count++;
    os_thread_sleep(SLEEP_BETWEEN_RETRIES_IN_US);

    MONITOR_INC(MONITOR_LOG_WRITER_ON_FREE_SPACE_WAITS);

    log_writer_mutex_enter(log);
  }

  return checkpoint_limited_lsn;
}

