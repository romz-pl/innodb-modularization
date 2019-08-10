#include <innodb/log_buffer/log_buffer_write_completed.h>

#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_buffer/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_sn/log_lsn_validate.h>
#include <innodb/log_types/log_t.h>
#include <innodb/monitor/MONITOR_INC_VALUE.h>

void log_buffer_write_completed(log_t &log, const Log_handle &handle,
                                lsn_t start_lsn, lsn_t end_lsn) {
  ut_ad(log.sn_lock.s_own(handle.lock_no));

  ut_a(log_lsn_validate(start_lsn));
  ut_a(log_lsn_validate(end_lsn));
  ut_a(end_lsn > start_lsn);

  /* Let M = log.recent_written_size (number of slots).
  For any integer k, all lsn values equal to: start_lsn + k*M
  correspond to the same slot, and only the smallest of them
  may use the slot. At most one of them can fit the range
  [log.buf_ready_for_write_lsn..log.buf_ready_ready_write_lsn+M).
  Any smaller values have already used the slot. Hence, we just
  need to wait until start_lsn will fit the mentioned range. */

  uint64_t wait_loops = 0;

  while (!log.recent_written.has_space(start_lsn)) {
    ++wait_loops;
    os_thread_sleep(20);
  }

  if (unlikely(wait_loops != 0)) {
    MONITOR_INC_VALUE(MONITOR_LOG_ON_RECENT_WRITTEN_WAIT_LOOPS, wait_loops);
  }

  /* Disallow reordering of writes to log buffer after this point.
  This is actually redundant, because we use seq_cst inside the
  log.recent_written.add_link(). However, we've decided to leave
  the seperate acq-rel synchronization between user threads and
  log writer. Reasons:
          1. Not to rely on internals of Link_buf::add_link.
          2. Stress that this synchronization is required in
             case someone decided to weaken memory ordering
             inside Link_buf. */
  std::atomic_thread_fence(std::memory_order_release);

  LOG_SYNC_POINT("log_buffer_write_completed_before_store");

  ut_ad(log.write_lsn.load() <= start_lsn);
  ut_ad(log_buffer_ready_for_write_lsn(log) <= start_lsn);

  /* Note that end_lsn will not point to just before footer,
  because we have already validated that end_lsn is valid. */
  log.recent_written.add_link(start_lsn, end_lsn);
}

