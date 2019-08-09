#include <innodb/log_write/log_write_up_to.h>

#include <innodb/io/srv_read_only_mode.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/log_wait_for_flush.h>
#include <innodb/log_write/log_wait_for_write.h>
#include <innodb/log_write/recv_no_ibuf_operations.h>
#include <innodb/log_write/srv_flush_log_at_trx_commit.h>

Wait_stats log_write_up_to(log_t &log, lsn_t end_lsn, bool flush_to_disk) {
  ut_a(!srv_read_only_mode);

  /* If we were updating log.flushed_to_disk_lsn while parsing redo log
  during recovery, we would have valid value here and we would not need
  to explicitly exit because of the recovery. However we do not update
  the log.flushed_to_disk during recovery (it is zero).

  On the other hand, when we apply log records during recovery, we modify
  pages and update their oldest/newest_modification. The modified pages
  become dirty. When size of the buffer pool is too small, some pages
  have to be flushed from LRU, to reclaim a free page for a next read.

  When flushing such dirty pages, we notice that newest_modification != 0,
  so the redo log has to be flushed up to the newest_modification, before
  flushing the page. In such case we end up here during recovery.

  Note that redo log is actually flushed, because changes to the page
  are caused by applying the redo. */

  if (recv_no_ibuf_operations) {
    /* Recovery is running and no operations on the log files are
    allowed yet, which is implicitly deduced from the fact, that
    still ibuf merges are disallowed. */
    return (Wait_stats{0});
  }

  /* We do not need to have exact numbers and we do not care if we
  lost some increments for heavy workload. The value only has usage
  when it is low workload and we need to discover that we request
  redo write or flush only from time to time. In such case we prefer
  to avoid spinning in log threads to save on CPU power usage. */
  log.write_to_file_requests_total.store(
      log.write_to_file_requests_total.load(std::memory_order_relaxed) + 1,
      std::memory_order_relaxed);

  ut_a(end_lsn != LSN_MAX);

  ut_a(end_lsn % OS_FILE_LOG_BLOCK_SIZE == 0 ||
       end_lsn % OS_FILE_LOG_BLOCK_SIZE >= LOG_BLOCK_HDR_SIZE);

  ut_a(end_lsn % OS_FILE_LOG_BLOCK_SIZE <=
       OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE);

  ut_ad(end_lsn <= log_get_lsn(log));

  if (flush_to_disk) {
    if (log.flushed_to_disk_lsn.load() >= end_lsn) {
      return (Wait_stats{0});
    }

    Wait_stats wait_stats{0};

    if (srv_flush_log_at_trx_commit != 1) {
      /* We need redo flushed, but because trx != 1, we have
      disabled notifications sent from log_writer to log_flusher.

      The log_flusher might be sleeping for 1 second, and we need
      quick response here. Log_writer avoids waking up log_flusher,
      so we must do it ourselves here.

      However, before we wake up log_flusher, we must ensure that
      log.write_lsn >= lsn. Otherwise log_flusher could flush some
      data which was ready for lsn values smaller than end_lsn and
      return to sleeping for next 1 second. */

      if (log.write_lsn.load() < end_lsn) {
        wait_stats = log_wait_for_write(log, end_lsn);
      }
    }

    /* Wait until log gets flushed up to end_lsn. */
    return (wait_stats + log_wait_for_flush(log, end_lsn));

  } else {
    if (log.write_lsn.load() >= end_lsn) {
      return (Wait_stats{0});
    }

    /* Wait until log gets written up to end_lsn. */
    return (log_wait_for_write(log, end_lsn));
  }
}
