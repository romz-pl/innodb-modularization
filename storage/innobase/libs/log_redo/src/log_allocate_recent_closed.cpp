#include <innodb/log_redo/log_allocate_recent_closed.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_redo/srv_log_recent_closed_size.h>

void log_allocate_recent_closed(log_t &log) {
  log.recent_closed = Link_buf<lsn_t>{srv_log_recent_closed_size};
}
