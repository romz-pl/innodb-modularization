#include <innodb/log_redo/log_advance_dirty_pages_added_up_to_lsn.h>

#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_buffer/log_buffer_dirty_pages_added_up_to_lsn.h>
#include <innodb/log_redo/log_closer_mutex_own.h>
#include <innodb/log_redo/log_closer_thread_active_validate.h>
#include <innodb/log_types/log_get_checkpoint_lsn.h>
#include <innodb/log_sn/log_lsn_validate.h>
#include <innodb/log_types/log_t.h>

bool log_advance_dirty_pages_added_up_to_lsn(log_t &log) {
  ut_ad(log_closer_mutex_own(log));

  const lsn_t previous_lsn = log_buffer_dirty_pages_added_up_to_lsn(log);

  ut_a(previous_lsn >= LOG_START_LSN);

  ut_a(previous_lsn >= log_get_checkpoint_lsn(log));

  ut_d(log_closer_thread_active_validate(log));

  auto stop_condition = [&](lsn_t prev_lsn, lsn_t next_lsn) {

    ut_a(log_lsn_validate(prev_lsn));
    ut_a(log_lsn_validate(next_lsn));

    ut_a(next_lsn > prev_lsn);

    LOG_SYNC_POINT("log_advance_dpa_before_update");
    return (false);
  };

  if (log.recent_closed.advance_tail_until(stop_condition)) {
    LOG_SYNC_POINT("log_advance_dpa_before_reclaim");

    /* Validation of recent_closed is optional because
    it takes significant time (delaying the log closer). */
    if (log_test != nullptr &&
        log_test->enabled(Log_test::Options::VALIDATE_RECENT_CLOSED)) {
      /* All links between ready_lsn and lsn have
      been traversed. The slots can't be re-used
      before we updated the tail. */
      log.recent_closed.validate_no_links(
          previous_lsn, log_buffer_dirty_pages_added_up_to_lsn(log));
    }

    ut_a(log_buffer_dirty_pages_added_up_to_lsn(log) > previous_lsn);

    std::atomic_thread_fence(std::memory_order_acquire);

    return (true);

  } else {
    ut_a(log_buffer_dirty_pages_added_up_to_lsn(log) == previous_lsn);

    return (false);
  }
}

