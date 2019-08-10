#include <innodb/log_write/Log_thread_waiting.h>

#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_redo/log_write_notifier_mutex_enter.h>
#include <innodb/log_redo/log_write_notifier_mutex_exit.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/log_write_notifier.h>
#include <innodb/log_write/srv_log_write_notifier_spin_delay.h>
#include <innodb/log_write/srv_log_write_notifier_timeout.h>
#include <innodb/monitor/MONITOR_INC_WAIT_STATS.h>
#include <innodb/sync_event/os_event_set.h>


void log_write_notifier(log_t *log_ptr) {
  ut_a(log_ptr != nullptr);
  ut_a(log_ptr->write_notifier_thread_alive.load());

  log_t &log = *log_ptr;
  lsn_t lsn = log.write_lsn.load() + 1;

  log_write_notifier_mutex_enter(log);

  Log_thread_waiting waiting{log, log.write_notifier_event,
                             srv_log_write_notifier_spin_delay,
                             srv_log_write_notifier_timeout};

  for (uint64_t step = 0;; ++step) {
    if (!log.writer_thread_alive.load()) {
      if (lsn > log.write_lsn.load()) {
        ut_a(lsn == log.write_lsn.load() + 1);
        break;
      }
    }

    LOG_SYNC_POINT("log_write_notifier_before_check");

    bool released = false;

    auto stop_condition = [&log, lsn, &released](bool wait) {

      LOG_SYNC_POINT("log_write_notifier_after_event_reset");
      if (released) {
        log_write_notifier_mutex_enter(log);
        released = false;
      }

      LOG_SYNC_POINT("log_write_notifier_before_check");

      if (log.write_lsn.load() >= lsn || !log.writer_thread_alive.load()) {
        return (true);
      }

      if (wait) {
        log_write_notifier_mutex_exit(log);
        released = true;
      }
      LOG_SYNC_POINT("log_write_notifier_before_wait");

      return (false);
    };

    const auto wait_stats = waiting.wait(stop_condition);

    MONITOR_INC_WAIT_STATS(MONITOR_LOG_WRITE_NOTIFIER_, wait_stats);

    LOG_SYNC_POINT("log_write_notifier_before_write_lsn");

    const lsn_t write_lsn = log.write_lsn.load();

    const lsn_t notified_up_to_lsn =
        ut_uint64_align_up(write_lsn, OS_FILE_LOG_BLOCK_SIZE);

    while (lsn <= notified_up_to_lsn) {
      const auto slot =
          (lsn - 1) / OS_FILE_LOG_BLOCK_SIZE & (log.write_events_size - 1);

      lsn += OS_FILE_LOG_BLOCK_SIZE;

      LOG_SYNC_POINT("log_write_notifier_before_notify");

      os_event_set(log.write_events[slot]);
    }

    lsn = write_lsn + 1;

    if (step % 1024 == 0) {
      log_write_notifier_mutex_exit(log);

      os_thread_sleep(0);

      log_write_notifier_mutex_enter(log);
    }
  }

  log.write_notifier_thread_alive.store(false);

  log_write_notifier_mutex_exit(log);
}

