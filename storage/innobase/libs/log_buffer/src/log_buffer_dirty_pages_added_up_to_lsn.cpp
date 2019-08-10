#include <innodb/log_buffer/log_buffer_dirty_pages_added_up_to_lsn.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/log_t.h>

lsn_t log_buffer_dirty_pages_added_up_to_lsn(const log_t &log) {
  return (log.recent_closed.tail());
}

#endif
