#pragma once

#include <algorithm>

#include <innodb/univ/univ.h>
#include <innodb/wait/Wait_stats.h>
#include <innodb/thread/os_thread_sleep.h>
#include <innodb/wait/UT_RELAX_CPU.h>


#ifndef UNIV_HOTBACKUP
/** Waits in loop until given condition is satisfied. It starts waiting
using spin loop with pauses and after reaching maximum iterations, it
switches to loop with sleeps. The sleep time is multiplied by two after
every k-sleeps, until it reaches 100ms (starting at provided value).
@param[in]	spins_limit		maximum iterations without sleep
@param[in]	sleep			initial sleep time in microseconds
@param[in]	condition		returns true when condition is
                                        satisfied
@return		number of loops with sleep that have been used */
template <typename Condition>
inline static Wait_stats ut_wait_for(uint64_t spins_limit, uint64_t sleep,
                                     Condition condition = {}) {
  uint64_t sleeps = 0, spins = 0;
#ifdef _WIN32
  uint64_t next_level = 64;
#else
  uint64_t next_level = 4;
#endif

  while (!condition(spins == spins_limit)) {
    if (spins == spins_limit) {
      ++sleeps;

      if (sleeps == next_level) {
        sleep = std::min(sleep * 2, uint64_t{100 * 1000});

#ifdef _WIN32
        next_level += 64;
#else
        next_level += 4;
#endif
      }

      os_thread_sleep(sleep);

    } else {
      ++spins;

      UT_RELAX_CPU();
    }
  }

  return (Wait_stats{sleeps});
}

#endif /* !UNIV_HOTBACKUP */
