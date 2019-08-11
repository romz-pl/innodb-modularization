#include <innodb/log_sys/log_allocate_recent_written.h>

#include <innodb/log_write/srv_log_recent_written_size.h>

#include <innodb/log_types/log_t.h>

void log_allocate_recent_written(log_t &log) {
  log.recent_written = Link_buf<lsn_t>{srv_log_recent_written_size};
}
