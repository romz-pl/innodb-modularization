#include <innodb/log_buffer/log_buffer_flush_order_lag.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/log_t.h>

lsn_t log_buffer_flush_order_lag(const log_t &log) {
  return (log.recent_closed.capacity());
}

#endif
