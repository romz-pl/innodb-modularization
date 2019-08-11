#include <innodb/log_chkp/log_checkpoint_time_elapsed.h>

#include <innodb/log_chkp/log_checkpointer_mutex_own.h>

#include <chrono>

uint64_t log_checkpoint_time_elapsed(const log_t &log) {
  ut_ad(log_checkpointer_mutex_own(log));

  const auto current_time = std::chrono::high_resolution_clock::now();

  const auto checkpoint_time = log.last_checkpoint_time;

  if (current_time < log.last_checkpoint_time) {
    return (0);
  }

  return (std::chrono::duration_cast<std::chrono::microseconds>(current_time -
                                                                checkpoint_time)
              .count());
}
