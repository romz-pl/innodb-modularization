#include <innodb/log_redo/log_background_write_threads_active_validate.h>

#include <innodb/log_types/log_t.h>

void log_background_write_threads_active_validate(const log_t &log) {
  ut_ad(!log.disable_redo_writes);

  ut_a(log.writer_thread_alive.load());

  ut_a(log.flusher_thread_alive.load());
}
