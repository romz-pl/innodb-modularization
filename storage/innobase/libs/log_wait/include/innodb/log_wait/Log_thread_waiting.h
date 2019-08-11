#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_wait/log_write_to_file_requests_are_frequent.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_wait/srv_cpu_usage.h>
#include <innodb/log_wait/srv_log_spin_cpu_abs_lwm.h>
#include <innodb/wait/Wait_stats.h>
#include <innodb/sync_event/os_event_wait_for.h>

/** Small utility which is used inside log threads when they have to
wait for next interesting event to happen. For performance reasons,
it might make sense to use spin-delay in front of the wait on event
in such cases. The strategy is first to spin and then to fallback to
the wait on event. However, for idle servers or work-loads which do
not need redo being flushed as often, we prefer to avoid spinning.
This utility solves such problems and provides waiting mechanism. */
struct Log_thread_waiting {
  Log_thread_waiting(const log_t &log, os_event_t event, uint64_t spin_delay,
                     uint64_t min_timeout)
      : m_log(log),
        m_event{event},
        m_spin_delay{static_cast<uint32_t>(std::min(
            uint64_t(std::numeric_limits<uint32_t>::max()), spin_delay))},
        m_min_timeout{static_cast<uint32_t>(
            /* No more than 1s */
            std::min(uint64_t{1000 * 1000}, min_timeout))} {}

  template <typename Stop_condition>
  inline Wait_stats wait(Stop_condition stop_condition) {
    auto spin_delay = m_spin_delay;
    auto min_timeout = m_min_timeout;

    /** We might read older value, it just decides on spinning.
    Correctness does not depend on this. Only local performance
    might depend on this but it's anyway heuristic and depends
    on average which by definition has lag. No reason to make
    extra barriers here. */

    const auto req_interval =
        m_log.write_to_file_requests_interval.load(std::memory_order_relaxed);

    if (srv_cpu_usage.utime_abs < srv_log_spin_cpu_abs_lwm ||
        !log_write_to_file_requests_are_frequent(req_interval)) {
      /* Either:
      1. CPU usage is very low on the server, which means the server
         is most likely idle or almost idle.
      2. Request to write/flush redo to disk comes only once per 1ms
         in average or even less often.
      In both cases we prefer not to spend on CPU power, because there
      is no real gain from spinning in log threads then. */

      spin_delay = 0;
      min_timeout =
          static_cast<uint32_t>(req_interval < 1000 ? req_interval : 1000);
    }

    const auto wait_stats =
        os_event_wait_for(m_event, spin_delay, min_timeout, stop_condition);

    return (wait_stats);
  }

 private:
  const log_t &m_log;
  os_event_t m_event;
  const uint32_t m_spin_delay;
  const uint32_t m_min_timeout;
};
