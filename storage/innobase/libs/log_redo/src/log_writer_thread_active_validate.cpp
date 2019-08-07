#include <innodb/log_redo/log_writer_thread_active_validate.h>

#include <innodb/log_types/log_t.h>

void log_writer_thread_active_validate(const log_t &log) {
  ut_a(log.writer_thread_alive.load());
}
