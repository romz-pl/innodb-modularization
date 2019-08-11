#include <innodb/log_flush/log_flush_notifier.h>

#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_flush/log_flush_notifier_mutex_enter.h>
#include <innodb/log_flush/log_flush_notifier_mutex_exit.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/Log_thread_waiting.h>
#include <innodb/log_flush/srv_log_flush_notifier_spin_delay.h>
#include <innodb/log_flush/srv_log_flush_notifier_timeout.h>
#include <innodb/monitor/MONITOR_INC_WAIT_STATS.h>
#include <innodb/sync_event/os_event_set.h>

void log_flush_notifier(log_t *log_ptr) {
  ut_a(log_ptr != nullptr);
  ut_a(log_ptr->flush_notifier_thread_alive.load());

  log_t &log = *log_ptr;
  lsn_t lsn = log.flushed_to_disk_lsn.load() + 1;

  log_flush_notifier_mutex_enter(log);

  Log_thread_waiting waiting{log, log.flush_notifier_event,
                             srv_log_flush_notifier_spin_delay,
                             srv_log_flush_notifier_timeout};

  for (uint64_t step = 0;; ++step) {
    if (!log.flusher_thread_alive.load()) {
      if (lsn > log.flushed_to_disk_lsn.load()) {
        ut_a(lsn == log.flushed_to_disk_lsn.load() + 1);
        break;
      }
    }

    LOG_SYNC_POINT("log_flush_notifier_before_check");

    bool released = false;

    auto stop_condition = [&log, lsn, &released](bool wait) {

      LOG_SYNC_POINT("log_flush_notifier_after_event_reset");
      if (released) {
        log_flush_notifier_mutex_enter(log);
        released = false;
      }

      LOG_SYNC_POINT("log_flush_notifier_before_check");

      if (log.flushed_to_disk_lsn.load() >= lsn ||
          !log.flusher_thread_alive.load()) {
        return (true);
      }

      if (wait) {
        log_flush_notifier_mutex_exit(log);
        released = true;
      }
      LOG_SYNC_POINT("log_flush_notifier_before_wait");

      return (false);
    };

    const auto wait_stats = waiting.wait(stop_condition);

    MONITOR_INC_WAIT_STATS(MONITOR_LOG_FLUSH_NOTIFIER_, wait_stats);

    LOG_SYNC_POINT("log_flush_notifier_before_flushed_to_disk_lsn");

    const lsn_t flush_lsn = log.flushed_to_disk_lsn.load();

    const lsn_t notified_up_to_lsn =
        ut_uint64_align_up(flush_lsn, OS_FILE_LOG_BLOCK_SIZE);

    while (lsn <= notified_up_to_lsn) {
      const auto slot =
          (lsn - 1) / OS_FILE_LOG_BLOCK_SIZE & (log.flush_events_size - 1);

      lsn += OS_FILE_LOG_BLOCK_SIZE;

      LOG_SYNC_POINT("log_flush_notifier_before_notify");

      os_event_set(log.flush_events[slot]);
    }

    lsn = flush_lsn + 1;

    if (step % 1024 == 0) {
      log_flush_notifier_mutex_exit(log);

      os_thread_sleep(0);

      log_flush_notifier_mutex_enter(log);
    }
  }

  log.flush_notifier_thread_alive.store(false);

  log_flush_notifier_mutex_exit(log);
}

