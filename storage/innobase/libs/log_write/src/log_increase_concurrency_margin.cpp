#include <innodb/log_write/log_increase_concurrency_margin.h>

#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/log_types/log_t.h>
#include <innodb/monitor/MONITOR_SET.h>

void log_increase_concurrency_margin(log_t &log) {
  /* Increase margin by 20% but do not exceed maximum allowed size. */
  const auto new_size =
      std::min(log.max_concurrency_margin,
               ut_uint64_align_up(
                   static_cast<lsn_t>(log.concurrency_margin.load() * 1.2),
                   OS_FILE_LOG_BLOCK_SIZE));

  log.concurrency_margin.store(new_size);

  MONITOR_SET(MONITOR_LOG_CONCURRENCY_MARGIN, new_size);
}
