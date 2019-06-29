#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/os_event_t.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_wait_time_low.h>
#include <innodb/wait/Wait_stats.h>
#include <innodb/wait/UT_RELAX_CPU.h>

#ifndef UNIV_HOTBACKUP


/** Waits in loop until a provided condition is satisfied. Combines usage
of spin-delay and event.

@remarks

First it uses a spin loop with PAUSE instructions. In each spin iteration
it checks the condition and stops as soon as it returned true.

When a provided number of spin iterations is reached, and the condition
still has not returned true, waiting on a provided event starts.

Each wait uses a provided timeout. After each wake-up the condition is
re-checked and function stops as soon as the condition returned true.

Every k-waits (ended on wake-up or timeout), the timeout is multiplied by two
(but it's limited up to maximum value of 100ms).

@param[in,out]	event			event on which function may wait
@param[in]	spins_limit		maximum spin iterations
@param[in]	timeout			initial timeout value
@param[in]	timeout_mul2_every	timeout is multiplied by two every
                                        that many waits on event
@param[in]	condition		returns true when condition is
                                        satisfied

@return number of loops with wait on event that have been used */
template <typename Condition>
inline static Wait_stats os_event_wait_for(os_event_t &event,
                                           uint64_t spins_limit,
                                           uint64_t timeout,
                                           Condition condition = {}) {
#ifdef _WIN32
  uint32_t next_level = 64;
#else
  uint32_t next_level = 4;
#endif
  uint32_t waits = 0;

  constexpr uint64_t MAX_TIMEOUT_US = 100 * 1000;

  while (true) {
    /* Store current sig_count before checking the
    condition, not to miss notification. */
    const bool wait = spins_limit == 0;

    const int64_t sig_count = !wait ? 0 : os_event_reset(event);

    /* Important: we do not want to split this loop to two
    loops (one for spin-delay and one for event), because
    we assume the condition is inlined below, and we don't
    want to make it inlined in two places. */

    if (condition(wait)) {
      return (Wait_stats{waits});
    }

    if (!wait) {
      /* It's still spin-delay loop. */
      --spins_limit;

      UT_RELAX_CPU();

    } else {
      /* Event-based loop. */
      ++waits;

      if (waits == next_level) {
        timeout = std::min(timeout * 2, MAX_TIMEOUT_US);

#ifdef _WIN32
        /* On Windows timeout is expressed in ms,
        so it's divided by 1000 and rounded down
        to 0 when it's smaller than 1000.

        In such case, it takes in average 10us to
        perform single SleepConditionVariableCS.

        So we need to perform more such 10us waits
        to simulate given number of timeout waits. */
        next_level += 64;
#else
        next_level += 4;
#endif
      }

      /* This translates to pthread_cond_wait (linux). */
      os_event_wait_time_low(event, timeout, sig_count);
    }
  }
}


#endif
