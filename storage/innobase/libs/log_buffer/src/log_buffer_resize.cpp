#include <innodb/log_buffer/log_buffer_resize.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/log_types/log_checkpointer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_buffer/log_buffer_resize_low.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/log_types/log_checkpointer_mutex_exit.h>
#include <innodb/log_types/log_buffer_x_lock_exit.h>
#include <innodb/log_types/log_buffer_x_lock_enter.h>

bool log_buffer_resize(log_t &log, size_t new_size) {
  log_buffer_x_lock_enter(log);

  const lsn_t end_lsn = log_get_lsn(log);

  log_checkpointer_mutex_enter(log);
  log_writer_mutex_enter(log);

  const bool ret = log_buffer_resize_low(log, new_size, end_lsn);

  log_writer_mutex_exit(log);
  log_checkpointer_mutex_exit(log);
  log_buffer_x_lock_exit(log);

  return (ret);
}
