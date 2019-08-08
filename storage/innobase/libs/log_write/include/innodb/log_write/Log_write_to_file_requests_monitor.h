#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_types/log_t.h>
#include <innodb/monitor/MONITOR_SET.h>

struct Log_write_to_file_requests_monitor {
  explicit Log_write_to_file_requests_monitor(log_t &log)
      : m_log(log), m_last_requests_value{0}, m_request_interval{0} {
    m_last_requests_time = Log_clock::now();
  }

  void update() {
    const auto requests_value =
        m_log.write_to_file_requests_total.load(std::memory_order_relaxed);

    const auto current_time = Log_clock::now();
    if (current_time < m_last_requests_time) {
      m_last_requests_time = current_time;
      return;
    }

    const auto delta_time = current_time - m_last_requests_time;
    const auto delta_time_us =
        std::chrono::duration_cast<std::chrono::microseconds>(delta_time)
            .count();

    if (requests_value > m_last_requests_value) {
      const auto delta_requests = requests_value - m_last_requests_value;
      const auto request_interval = delta_time_us / delta_requests;
      m_request_interval = (m_request_interval * 63 + request_interval) / 64;

    } else if (delta_time_us > 100 * 1000) {
      /* Last call to log_write_up_to() was longer than 100ms ago,
      so consider this as maximum time between calls we can expect.
      Tracking higher values does not make sense, because it is for
      sure already higher than any reasonable threshold which can be
      used to differ different activity modes. */

      m_request_interval = 100 * 1000; /* 100ms */

    } else {
      /* No progress in number of requests and still no more than
      1second since last progress. Postpone any decision. */
      return;
    }

    m_log.write_to_file_requests_interval.store(m_request_interval,
                                                std::memory_order_relaxed);

    MONITOR_SET(MONITOR_LOG_WRITE_TO_FILE_REQUESTS_INTERVAL,
                m_request_interval);

    m_last_requests_time = current_time;
    m_last_requests_value = requests_value;
  }

 private:
  log_t &m_log;
  uint64_t m_last_requests_value;
  Log_clock_point m_last_requests_time;
  uint64_t m_request_interval;
};
