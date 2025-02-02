#include <innodb/log_write/log_position_unlock.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_chkp/log_checkpointer_mutex_exit.h>
#include <innodb/log_types/log_buffer_x_lock_exit.h>

void log_position_unlock(log_t &log) {
  log_checkpointer_mutex_exit(log);

  log_buffer_x_lock_exit(log);
}
