#include <innodb/log_buffer/log_buffer_ready_for_write_lsn.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/log_t.h>

lsn_t log_buffer_ready_for_write_lsn(const log_t &log) {
  return (log.recent_written.tail());
}

#endif
