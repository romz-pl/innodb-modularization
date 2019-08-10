#include <innodb/log_redo/log_position_lock.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_chkp/log_checkpointer_mutex_enter.h>
#include <innodb/log_types/log_buffer_x_lock_enter.h>

void log_position_lock(log_t &log) {
  log_buffer_x_lock_enter(log);

  log_checkpointer_mutex_enter(log);
}
