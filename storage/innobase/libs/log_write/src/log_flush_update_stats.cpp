#include <innodb/log_write/log_flush_update_stats.h>

#include <innodb/log_redo/log_flusher_mutex_own.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/srv_flushing_avg_loops.h>
#include <innodb/monitor/MONITOR_INC_VALUE.h>
#include <innodb/monitor/MONITOR_SET.h>

void log_flush_update_stats(log_t &log) {
  ut_ad(log_flusher_mutex_own(log));

  /* Note that this code is inspired by similar logic in buf0flu.cc */

  static uint64_t iterations = 0;
  static Log_clock_point prev_time{};
  static lsn_t prev_lsn;
  static lsn_t lsn_avg_rate = 0;
  static Log_clock::duration fsync_max_time;
  static Log_clock::duration fsync_total_time;

  /* Calculate time of last fsync and update related counters. */

  Log_clock::duration fsync_time;

  fsync_time = log.last_flush_end_time - log.last_flush_start_time;

  ut_a(fsync_time.count() >= 0);

  fsync_max_time = std::max(fsync_max_time, fsync_time);

  fsync_total_time += fsync_time;

  MONITOR_INC_VALUE(
      MONITOR_LOG_FLUSH_TOTAL_TIME,
      std::chrono::duration_cast<std::chrono::milliseconds>(fsync_time)
          .count());

  /* Calculate time elapsed since start of last sample. */

  if (prev_time == Log_clock_point{}) {
    prev_time = log.last_flush_start_time;
    prev_lsn = log.flushed_to_disk_lsn.load();
  }

  const Log_clock_point curr_time = log.last_flush_end_time;

  if (curr_time < prev_time) {
    /* Time was moved backward since we set prev_time.
    We cannot determine how much time passed since then. */
    prev_time = curr_time;
  }

  auto time_elapsed =
      std::chrono::duration_cast<std::chrono::seconds>(curr_time - prev_time)
          .count();

  ut_a(time_elapsed >= 0);

  if (++iterations >= srv_flushing_avg_loops ||
      time_elapsed >= static_cast<double>(srv_flushing_avg_loops)) {
    if (time_elapsed < 1) {
      time_elapsed = 1;
    }

    const lsn_t curr_lsn = log.flushed_to_disk_lsn.load();

    const lsn_t lsn_rate = static_cast<lsn_t>(
        static_cast<double>(curr_lsn - prev_lsn) / time_elapsed);

    lsn_avg_rate = (lsn_avg_rate + lsn_rate) / 2;

    MONITOR_SET(MONITOR_LOG_FLUSH_LSN_AVG_RATE, lsn_avg_rate);

    MONITOR_SET(
        MONITOR_LOG_FLUSH_MAX_TIME,
        std::chrono::duration_cast<std::chrono::microseconds>(fsync_max_time)
            .count());

    log.flush_avg_time =
        std::chrono::duration_cast<std::chrono::microseconds>(fsync_total_time)
            .count() *
        1.0 / iterations;

    MONITOR_SET(MONITOR_LOG_FLUSH_AVG_TIME, log.flush_avg_time);

    fsync_max_time = Log_clock::duration{};
    fsync_total_time = Log_clock::duration{};
    iterations = 0;
    prev_time = curr_time;
    prev_lsn = curr_lsn;
  }
}

