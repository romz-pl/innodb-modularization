#include <innodb/log_redo/log_closer_thread_active_validate.h>

#include <innodb/log_types/log_t.h>

void log_closer_thread_active_validate(const log_t &log) {
  ut_a(log.closer_thread_alive.load());
}
