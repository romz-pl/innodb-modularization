#include <innodb/log_write/log_writer.h>

#include <innodb/log_redo/log_advance_ready_for_write_lsn.h>
#include <innodb/log_redo/log_advance_ready_for_write_lsn.h>
#include <innodb/log_buffer/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_buffer/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/Log_thread_waiting.h>
#include <innodb/log_write/Log_write_to_file_requests_monitor.h>
#include <innodb/log_write/srv_log_writer_spin_delay.h>
#include <innodb/log_write/srv_log_writer_timeout.h>
#include <innodb/monitor/MONITOR_INC_WAIT_STATS.h>
#include <innodb/sync_event/os_event_set.h>
//#include <innodb/log_write/log_writer_write_buffer.h>

void log_writer_write_buffer(log_t &log, lsn_t next_write_lsn);

void log_writer(log_t *log_ptr) {
  ut_a(log_ptr != nullptr);
  ut_a(log_ptr->writer_thread_alive.load());

  log_t &log = *log_ptr;
  lsn_t ready_lsn = 0;

  log_writer_mutex_enter(log);

  Log_thread_waiting waiting{log, log.writer_event, srv_log_writer_spin_delay,
                             srv_log_writer_timeout};

  Log_write_to_file_requests_monitor write_to_file_requests_monitor{log};

  for (uint64_t step = 0;; ++step) {
    bool released = false;

    auto stop_condition = [&ready_lsn, &log, &released,
                           &write_to_file_requests_monitor](bool wait) {

      if (released) {
        log_writer_mutex_enter(log);
        released = false;
      }

      /* Advance lsn up to which data is ready in log buffer. */
      (void)log_advance_ready_for_write_lsn(log);

      ready_lsn = log_buffer_ready_for_write_lsn(log);

      /* Wait until any of following conditions holds:
              1) There is some unwritten data in log buffer
              2) We should close threads. */

      if (log.write_lsn.load() < ready_lsn || log.should_stop_threads.load()) {
        return (true);
      }

      if (wait) {
        write_to_file_requests_monitor.update();
        log_writer_mutex_exit(log);
        released = true;
      }

      return (false);
    };

    const auto wait_stats = waiting.wait(stop_condition);

    MONITOR_INC_WAIT_STATS(MONITOR_LOG_WRITER_, wait_stats);

    /* Do the actual work. */
    if (log.write_lsn.load() < ready_lsn) {
      log_writer_write_buffer(log, ready_lsn);

      if (step % 1024 == 0) {
        write_to_file_requests_monitor.update();

        log_writer_mutex_exit(log);

        os_thread_sleep(0);

        log_writer_mutex_enter(log);
      }

    } else {
      if (log.should_stop_threads.load()) {
        /* When log threads are stopped, we must first
        ensure that all writes to log buffer have been
        finished and only then we are allowed to set
        the should_stop_threads to true. */

        if (!log_advance_ready_for_write_lsn(log)) {
          break;
        }

        ready_lsn = log_buffer_ready_for_write_lsn(log);
      }
    }
  }

  log.writer_thread_alive.store(false);

  os_event_set(log.write_notifier_event);
  os_event_set(log.flusher_event);

  log_writer_mutex_exit(log);
}

