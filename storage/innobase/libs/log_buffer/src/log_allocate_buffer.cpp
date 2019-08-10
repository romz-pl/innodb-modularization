#include <innodb/log_buffer/log_allocate_buffer.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_buffer/srv_log_buffer_size.h>
#include <innodb/assert/assert.h>

void log_allocate_buffer(log_t &log) {
  ut_a(srv_log_buffer_size >= INNODB_LOG_BUFFER_SIZE_MIN);
  ut_a(srv_log_buffer_size <= INNODB_LOG_BUFFER_SIZE_MAX);
  ut_a(srv_log_buffer_size >= 4 * UNIV_PAGE_SIZE);

  log.buf.create(srv_log_buffer_size);
}
