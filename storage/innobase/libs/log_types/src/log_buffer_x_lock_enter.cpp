#include <innodb/log_types/log_buffer_x_lock_enter.h>

#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_types/log_t.h>

void log_buffer_x_lock_enter(log_t &log) {
  LOG_SYNC_POINT("log_buffer_x_lock_enter_before_lock");

  log.sn_lock.x_lock();

  LOG_SYNC_POINT("log_buffer_x_lock_enter_after_lock");
}
