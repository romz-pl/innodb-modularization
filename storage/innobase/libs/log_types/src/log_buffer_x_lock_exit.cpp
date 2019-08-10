#include <innodb/log_types/log_buffer_x_lock_exit.h>


#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_types/log_t.h>

void log_buffer_x_lock_exit(log_t &log) {
  LOG_SYNC_POINT("log_buffer_x_lock_exit_before_unlock");

  log.sn_lock.x_unlock();

  LOG_SYNC_POINT("log_buffer_x_lock_exit_after_unlock");
}
