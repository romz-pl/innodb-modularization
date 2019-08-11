#include <innodb/log_sys/log_stop_background_threads.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_write/log_background_threads_active_validate.h>
#include <innodb/log_write/log_background_threads_inactive_validate.h>
#include <innodb/sync_event/os_event_set.h>
#include <innodb/logger/info.h>

extern bool srv_read_only_mode;

void log_stop_background_threads(log_t &log) {
  /* We cannot stop threads when x-lock is acquired, because of scenario:
          * log_checkpointer starts log_checkpoint()
          * log_checkpoint() asks to persist dd dynamic metadata
          * dict_persist_dd_table_buffer() tries to write to redo
          * but cannot acquire shared lock on log.sn_lock
          * so log_checkpointer thread waits for this thread
            until the x-lock is released
          * but this thread waits until log background threads
            have been stopped - log_checkpointer is not stopped. */
  ut_ad(!log.sn_lock.x_own());

  ib::info(ER_IB_MSG_1259) << "Log background threads are being closed...";

  std::atomic_thread_fence(std::memory_order_seq_cst);

  log_background_threads_active_validate(log);

  ut_a(!srv_read_only_mode);

  log.should_stop_threads.store(true);

  /* Log writer may wait on writer_event with 100ms timeout, so we better
  wake him up, so he could notice that log.should_stop_threads has been
  set to true, finish his work and exit. */
  os_event_set(log.writer_event);

  /* The same applies to log_checkpointer thread and log_closer thread.
  However, it does not apply to others, because:
    - log_flusher monitors log.writer_thread_alive,
    - log_write_notifier monitors log.writer_thread_alive,
    - log_flush_notifier monitors log.flusher_thread_alive. */
  os_event_set(log.closer_event);
  os_event_set(log.checkpointer_event);

  /* Wait until threads are closed. */
  while (log.closer_thread_alive.load() ||
         log.checkpointer_thread_alive.load() ||
         log.writer_thread_alive.load() || log.flusher_thread_alive.load() ||
         log.write_notifier_thread_alive.load() ||
         log.flush_notifier_thread_alive.load()) {
    os_thread_sleep(100 * 1000);
  }

  std::atomic_thread_fence(std::memory_order_seq_cst);

  log_background_threads_inactive_validate(log);
}

