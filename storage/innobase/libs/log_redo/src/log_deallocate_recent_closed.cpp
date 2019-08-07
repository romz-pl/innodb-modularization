#include <innodb/log_redo/log_deallocate_recent_closed.h>

#include <innodb/log_types/log_t.h>

void log_deallocate_recent_closed(log_t &log) {
  log.recent_closed.validate_no_links();
  log.recent_closed = {};
}
