#include <innodb/log_sys/log_deallocate_write_ahead_buffer.h>

#include <innodb/log_types/log_t.h>

void log_deallocate_write_ahead_buffer(log_t &log) {
  log.write_ahead_buf.destroy();
}
