#include <innodb/log_write/log_threads_active.h>

#include <innodb/log_types/log_t.h>

bool log_threads_active(const log_t &log) {
  return (log.closer_thread_alive.load() ||
          log.checkpointer_thread_alive.load() ||
          log.writer_thread_alive.load() || log.flusher_thread_alive.load() ||
          log.write_notifier_thread_alive.load() ||
          log.flush_notifier_thread_alive.load());
}
