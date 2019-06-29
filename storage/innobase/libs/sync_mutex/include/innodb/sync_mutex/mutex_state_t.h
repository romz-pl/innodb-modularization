#pragma once

#include <innodb/univ/univ.h>

/** The new (C++11) syntax allows the following and we should use it when it
is available on platforms that we support.

        enum class mutex_state_t : lock_word_t { ... };
*/

/** Mutex states. */
enum mutex_state_t {
  /** Mutex is free */
  MUTEX_STATE_UNLOCKED = 0,

  /** Mutex is acquired by some thread. */
  MUTEX_STATE_LOCKED = 1,

  /** Mutex is contended and there are threads waiting on the lock. */
  MUTEX_STATE_WAITERS = 2
};
