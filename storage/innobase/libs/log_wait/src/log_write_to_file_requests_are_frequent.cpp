#include <innodb/log_wait/log_write_to_file_requests_are_frequent.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_types/log_t.h>

bool log_write_to_file_requests_are_frequent(uint64_t interval) {
  return (interval < 1000); /* 1ms */
}

bool log_write_to_file_requests_are_frequent(const log_t &log) {
  return (log_write_to_file_requests_are_frequent(
      log.write_to_file_requests_interval.load(std::memory_order_relaxed)));
}

#endif
