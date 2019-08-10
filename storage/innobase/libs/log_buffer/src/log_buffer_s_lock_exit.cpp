#include <innodb/log_buffer/log_buffer_s_lock_exit.h>

#include <innodb/log_types/log_t.h>

void log_buffer_s_lock_exit(log_t &log, size_t lock_no) {
  log.sn_lock.s_unlock(lock_no);
}
