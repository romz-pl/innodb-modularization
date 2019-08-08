#include <innodb/log_redo/log_advance_ready_for_write_lsn.h>

#include <innodb/log_redo/LOG_SYNC_POINT.h>
#include <innodb/log_redo/flags.h>
#include <innodb/log_redo/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_redo/log_lsn_validate.h>
#include <innodb/log_redo/log_test.h>
#include <innodb/log_redo/log_writer_mutex_own.h>
#include <innodb/log_redo/log_writer_thread_active_validate.h>
#include <innodb/log_redo/srv_log_write_max_size.h>
#include <innodb/log_types/log_t.h>

bool log_advance_ready_for_write_lsn(log_t &log) {
  ut_ad(log_writer_mutex_own(log));
  ut_d(log_writer_thread_active_validate(log));

  const lsn_t write_lsn = log.write_lsn.load();

  const auto write_max_size = srv_log_write_max_size;

  ut_a(write_max_size > 0);

  auto stop_condition = [&](lsn_t prev_lsn, lsn_t next_lsn) {

    ut_a(log_lsn_validate(prev_lsn));
    ut_a(log_lsn_validate(next_lsn));

    ut_a(next_lsn > prev_lsn);
    ut_a(prev_lsn >= write_lsn);

    LOG_SYNC_POINT("log_advance_ready_for_write_before_reclaim");

    return (prev_lsn - write_lsn >= write_max_size);
  };

  const lsn_t previous_lsn = log_buffer_ready_for_write_lsn(log);

  ut_a(previous_lsn >= write_lsn);

  if (log.recent_written.advance_tail_until(stop_condition)) {
    LOG_SYNC_POINT("log_advance_ready_for_write_before_update");

    /* Validation of recent_written is optional because
    it takes significant time (delaying the log writer). */
    if (log_test != nullptr &&
        log_test->enabled(Log_test::Options::VALIDATE_RECENT_WRITTEN)) {
      /* All links between ready_lsn and lsn have
      been traversed. The slots can't be re-used
      before we updated the tail. */
      log.recent_written.validate_no_links(previous_lsn,
                                           log_buffer_ready_for_write_lsn(log));
    }

    ut_a(log_buffer_ready_for_write_lsn(log) > previous_lsn);

    std::atomic_thread_fence(std::memory_order_acquire);

    return (true);

  } else {
    ut_a(log_buffer_ready_for_write_lsn(log) == previous_lsn);

    return (false);
  }
}
