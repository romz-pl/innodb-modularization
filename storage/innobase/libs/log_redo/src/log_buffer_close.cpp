#include <innodb/log_redo/log_buffer_close.h>

#include <innodb/log_redo/LOG_SYNC_POINT.h>
#include <innodb/log_redo/log_buffer_dirty_pages_added_up_to_lsn.h>
#include <innodb/log_redo/log_buffer_s_lock_exit.h>
#include <innodb/log_redo/log_lsn_validate.h>
#include <innodb/log_types/Log_handle.h>
#include <innodb/log_types/log_t.h>

void log_buffer_close(log_t &log, const Log_handle &handle) {
  const lsn_t start_lsn = handle.start_lsn;
  const lsn_t end_lsn = handle.end_lsn;

  ut_a(log_lsn_validate(start_lsn));
  ut_a(log_lsn_validate(end_lsn));
  ut_a(end_lsn > start_lsn);

  ut_ad(start_lsn >= log_buffer_dirty_pages_added_up_to_lsn(log));

  ut_ad(log.sn_lock.s_own(handle.lock_no));

  std::atomic_thread_fence(std::memory_order_release);

  LOG_SYNC_POINT("log_buffer_write_completed_dpa_before_store");

  log.recent_closed.add_link(start_lsn, end_lsn);

  log_buffer_s_lock_exit(log, handle.lock_no);
}
