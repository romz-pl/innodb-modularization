#include <innodb/log_write/log_closer.h>

#include <innodb/log_redo/log_advance_dirty_pages_added_up_to_lsn.h>
#include <innodb/log_redo/log_buffer_dirty_pages_added_up_to_lsn.h>
#include <innodb/log_redo/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_redo/log_closer_mutex_enter.h>
#include <innodb/log_redo/log_closer_mutex_exit.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/log_types/log_lsn_validate.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/Log_thread_waiting.h>
#include <innodb/log_write/srv_log_closer_spin_delay.h>
#include <innodb/log_write/srv_log_closer_timeout.h>
#include <innodb/sync_event/os_event_set.h>

void log_closer(log_t *log_ptr) {
  ut_a(log_ptr != nullptr);
  ut_a(log_ptr->closer_thread_alive.load());

  log_t &log = *log_ptr;
  lsn_t end_lsn = 0;

  log_closer_mutex_enter(log);

  Log_thread_waiting waiting{log, log.closer_event, srv_log_closer_spin_delay,
                             srv_log_closer_timeout};

  for (uint64_t step = 0;; ++step) {
    bool released = false;

    auto stop_condition = [&log, &released, step](bool wait) {

      if (released) {
        log_closer_mutex_enter(log);
        released = false;
      }

      /* Advance lsn up to which all the dirty pages have
      been added to flush lists. */

      if (log_advance_dirty_pages_added_up_to_lsn(log)) {
        if (step % 1024 == 0) {
          log_closer_mutex_exit(log);
          os_thread_sleep(0);
          log_closer_mutex_enter(log);
        }
        return (true);
      }

      if (log.should_stop_threads.load()) {
        return (true);
      }

      if (wait) {
        log_closer_mutex_exit(log);
        released = true;
      }
      return (false);
    };

    waiting.wait(stop_condition);

    /* Check if we should close the thread. */
    if (log.should_stop_threads.load() && !log.flusher_thread_alive.load() &&
        !log.writer_thread_alive.load()) {
      end_lsn = log.write_lsn.load();

      ut_a(log_lsn_validate(end_lsn));
      ut_a(end_lsn == log.flushed_to_disk_lsn.load());
      ut_a(end_lsn == log_buffer_ready_for_write_lsn(log));

      ut_a(end_lsn >= log_buffer_dirty_pages_added_up_to_lsn(log));

      if (log_buffer_dirty_pages_added_up_to_lsn(log) == end_lsn) {
        /* All confirmed reservations have been written
        to redo and all dirty pages related to those
        writes have been added to flush lists.

        However, there could be user threads, which are
        in the middle of log_buffer_reserve(), reserved
        range of sn values, but could not confirm.

        Note that because log_writer is already not alive,
        the only possible reason guaranteed by its death,
        is that there is x-lock at end_lsn, in which case
        end_lsn separates two regions in log buffer:
        completely full and completely empty. */
        const lsn_t ready_lsn = log_buffer_ready_for_write_lsn(log);

        const lsn_t current_lsn = log_get_lsn(log);

        if (current_lsn > ready_lsn) {
          log.recent_written.validate_no_links(ready_lsn, current_lsn);

          log.recent_closed.validate_no_links(ready_lsn, current_lsn);
        }

        break;
      }

      /* We need to wait until remaining dirty pages
      have been added. */
    }
  }

  log.closer_thread_alive.store(false);

  os_event_set(log.checkpointer_event);

  log_closer_mutex_exit(log);
}
