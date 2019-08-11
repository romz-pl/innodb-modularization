#include <innodb/log_sys/log_allocate_write_ahead_buffer.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_write/srv_log_write_ahead_size.h>
#include <innodb/assert/assert.h>

void log_allocate_write_ahead_buffer(log_t &log) {
  ut_a(srv_log_write_ahead_size >= INNODB_LOG_WRITE_AHEAD_SIZE_MIN);
  ut_a(srv_log_write_ahead_size <= INNODB_LOG_WRITE_AHEAD_SIZE_MAX);

  log.write_ahead_buf_size = srv_log_write_ahead_size;
  log.write_ahead_buf.create(log.write_ahead_buf_size);
}
