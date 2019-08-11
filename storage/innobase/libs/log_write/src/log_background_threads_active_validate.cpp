#include <innodb/log_write/log_background_threads_active_validate.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_write/log_background_write_threads_active_validate.h>

void log_background_threads_active_validate(const log_t &log) {
  log_background_write_threads_active_validate(log);

  ut_a(log.write_notifier_thread_alive.load());
  ut_a(log.flush_notifier_thread_alive.load());

  ut_a(log.closer_thread_alive.load());

  ut_a(log.checkpointer_thread_alive.load());
}
