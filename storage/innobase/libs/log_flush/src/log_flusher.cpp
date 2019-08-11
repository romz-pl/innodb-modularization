#include <innodb/log_flush/log_flusher.h>

#include <innodb/sync_event/os_event_set.h>
#include <innodb/monitor/MONITOR_INC_WAIT_STATS.h>
#include <innodb/sync_event/os_event_wait_time_low.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/Log_thread_waiting.h>
#include <innodb/log_flush/log_flusher_mutex_enter.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_flush/log_flush_low.h>
#include <innodb/log_flush/log_flusher_mutex_exit.h>
#include <innodb/log_flush/srv_flush_log_at_trx_commit.h>
#include <innodb/log_flush/srv_log_flusher_spin_delay.h>
#include <innodb/log_flush/srv_log_flusher_timeout.h>
#include <innodb/log_flush/srv_flush_log_at_timeout.h>

void log_flusher(log_t *log_ptr) {
  ut_a(log_ptr != nullptr);
  ut_a(log_ptr->flusher_thread_alive.load());

  log_t &log = *log_ptr;

  Log_thread_waiting waiting{log, log.flusher_event, srv_log_flusher_spin_delay,
                             srv_log_flusher_timeout};

  log_flusher_mutex_enter(log);

  for (uint64_t step = 0; log.writer_thread_alive.load(); ++step) {
    bool released = false;

    auto stop_condition = [&log, &released, step](bool wait) {

      if (released) {
        log_flusher_mutex_enter(log);
        released = false;
      }

      LOG_SYNC_POINT("log_flusher_before_should_flush");

      const lsn_t last_flush_lsn = log.flushed_to_disk_lsn.load();

      ut_a(last_flush_lsn <= log.write_lsn.load());

      if (last_flush_lsn < log.write_lsn.load()) {
        /* Flush and stop waiting. */
        log_flush_low(log);

        if (step % 1024 == 0) {
          log_flusher_mutex_exit(log);

          os_thread_sleep(0);

          log_flusher_mutex_enter(log);
        }

        return (true);
      }

      /* Stop waiting if writer thread is dead. */
      if (!log.writer_thread_alive.load()) {
        return (true);
      }

      if (wait) {
        log_flusher_mutex_exit(log);
        released = true;
      }

      return (false);
    };

    if (srv_flush_log_at_trx_commit != 1) {
      const auto current_time = Log_clock::now();

      ut_ad(log.last_flush_end_time >= log.last_flush_start_time);

      if (current_time < log.last_flush_end_time) {
        /* Time was moved backward, possibly by a lot, so we need to
        adjust the last_flush times, because otherwise we could stop
        flushing every innodb_flush_log_at_timeout for a while. */
        log.last_flush_start_time = current_time;
        log.last_flush_end_time = current_time;
      }

      const auto time_elapsed = current_time - log.last_flush_start_time;

      using us = std::chrono::microseconds;

      const auto time_elapsed_us =
          std::chrono::duration_cast<us>(time_elapsed).count();

      ut_a(time_elapsed_us >= 0);

      const auto flush_every = srv_flush_log_at_timeout;

      const auto flush_every_us = 1000000LL * flush_every;

      if (time_elapsed_us < flush_every_us) {
        log_flusher_mutex_exit(log);

        if (log.writer_thread_alive.load()) {
          /* If log.writer_thread_alive became false just now, we would
          have the flusher_event set, because: the only place where we
          can reset is just before fsync and after writer_thread_alive
          is set to false, the flusher_event is set. */

          os_event_wait_time_low(log.flusher_event,
                                 flush_every_us - time_elapsed_us, 0);
        }

        log_flusher_mutex_enter(log);
      }
    }

    const auto wait_stats = waiting.wait(stop_condition);

    MONITOR_INC_WAIT_STATS(MONITOR_LOG_FLUSHER_, wait_stats);
  }

  if (log.write_lsn.load() > log.flushed_to_disk_lsn.load()) {
    log_flush_low(log);
  }

  log.flusher_thread_alive.store(false);

  os_event_set(log.flush_notifier_event);

  log_flusher_mutex_exit(log);
}

