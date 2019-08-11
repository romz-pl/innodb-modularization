#include <innodb/log_write/log_wait_for_space_in_log_recent_closed.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_sn/log_lsn_validate.h>
#include <innodb/log_buffer/log_buffer_dirty_pages_added_up_to_lsn.h>
#include <innodb/monitor/MONITOR_INC_VALUE.h>

void log_wait_for_space_in_log_recent_closed(log_t &log, lsn_t lsn) {
  ut_a(log_lsn_validate(lsn));

  ut_ad(lsn >= log_buffer_dirty_pages_added_up_to_lsn(log));

  uint64_t wait_loops = 0;

  while (!log.recent_closed.has_space(lsn)) {
    ++wait_loops;
    os_thread_sleep(20);
  }

  if (unlikely(wait_loops != 0)) {
    MONITOR_INC_VALUE(MONITOR_LOG_ON_RECENT_CLOSED_WAIT_LOOPS, wait_loops);
  }
}
