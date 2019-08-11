#include <innodb/log_write/log_background_threads_inactive_validate.h>

#include <innodb/log_types/log_t.h>

void log_background_threads_inactive_validate(const log_t &log) {
  ut_a(!log.checkpointer_thread_alive.load());
  ut_a(!log.closer_thread_alive.load());
  ut_a(!log.write_notifier_thread_alive.load());
  ut_a(!log.flush_notifier_thread_alive.load());
  ut_a(!log.writer_thread_alive.load());
  ut_a(!log.flusher_thread_alive.load());
}
