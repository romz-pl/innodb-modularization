#include <innodb/log_redo/log_deallocate_recent_written.h>

#include <innodb/log_types/log_t.h>

void log_deallocate_recent_written(log_t &log) {
  log.recent_written.validate_no_links();
  log.recent_written = {};
}
