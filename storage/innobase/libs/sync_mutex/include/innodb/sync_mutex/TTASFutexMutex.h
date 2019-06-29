#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifdef HAVE_IB_LINUX_FUTEX

#include <innodb/atomic/atomic.h>
#include <innodb/random/random.h>
#include <innodb/sync_mutex/mutex_state_t.h>
#include <innodb/sync_os/OSMutex.h>
#include <innodb/sync_policy/NoPolicy.h>
#include <innodb/wait/ut_delay.h>

#include <atomic>
#include <linux/futex.h>
#include <sys/syscall.h>

/** Mutex implementation that used the Linux futex. */
template <template <typename> class Policy = NoPolicy>
struct TTASFutexMutex {
  typedef Policy<TTASFutexMutex> MutexPolicy;

  TTASFutexMutex() UNIV_NOTHROW : m_lock_word(MUTEX_STATE_UNLOCKED) {
    /* Check that lock_word is aligned. */
    ut_ad(!((ulint)&m_lock_word % sizeof(ulint)));
  }

  ~TTASFutexMutex() { ut_a(m_lock_word == MUTEX_STATE_UNLOCKED); }

  /** Called when the mutex is "created". Note: Not from the constructor
  but when the mutex is initialised.
  @param[in]	id		Mutex ID
  @param[in]	filename	File where mutex was created
  @param[in]	line		Line in filename */
  void init(latch_id_t id, const char *filename, uint32_t line) UNIV_NOTHROW {
    ut_a(m_lock_word == MUTEX_STATE_UNLOCKED);
    m_policy.init(*this, id, filename, line);
  }

  /** Destroy the mutex. */
  void destroy() UNIV_NOTHROW {
    /* The destructor can be called at shutdown. */
    ut_a(m_lock_word == MUTEX_STATE_UNLOCKED);
    m_policy.destroy();
  }

  /** Acquire the mutex.
  @param[in]	max_spins	max number of spins
  @param[in]	max_delay	max delay per spin
  @param[in]	filename	from where called
  @param[in]	line		within filename */
  void enter(uint32_t max_spins, uint32_t max_delay, const char *filename,
             uint32_t line) UNIV_NOTHROW {
    uint32_t n_spins;
    lock_word_t lock = ttas(max_spins, max_delay, n_spins);

    /* If there were no waiters when this thread tried
    to acquire the mutex then set the waiters flag now.
    Additionally, when this thread set the waiters flag it is
    possible that the mutex had already been released
    by then. In this case the thread can assume it
    was granted the mutex. */

    uint32_t n_waits;

    if (lock != MUTEX_STATE_UNLOCKED) {
      if (lock != MUTEX_STATE_LOCKED || !set_waiters()) {
        n_waits = wait();
      } else {
        n_waits = 0;
      }

    } else {
      n_waits = 0;
    }

    m_policy.add(n_spins, n_waits);
  }

  /** Release the mutex. */
  void exit() UNIV_NOTHROW {
    /* If there are threads waiting then we have to wake
    them up. Reset the lock state to unlocked so that waiting
    threads can test for success. */

    std::atomic_thread_fence(std::memory_order_acquire);

    if (state() == MUTEX_STATE_WAITERS) {
      m_lock_word = MUTEX_STATE_UNLOCKED;

    } else if (unlock() == MUTEX_STATE_LOCKED) {
      /* No threads waiting, no need to signal a wakeup. */
      return;
    }

    signal();
  }

  /** Try and lock the mutex.
  @return the old state of the mutex */
  lock_word_t trylock() UNIV_NOTHROW {
    return (CAS(&m_lock_word, MUTEX_STATE_UNLOCKED, MUTEX_STATE_LOCKED));
  }

  /** Try and lock the mutex.
  @return true if successful */
  bool try_lock() UNIV_NOTHROW { return (trylock() == MUTEX_STATE_UNLOCKED); }

  /** @return true if mutex is unlocked */
  bool is_locked() const UNIV_NOTHROW {
    return (state() != MUTEX_STATE_UNLOCKED);
  }

#ifdef UNIV_DEBUG
  /** @return true if the thread owns the mutex. */
  bool is_owned() const UNIV_NOTHROW {
    return (is_locked() && m_policy.is_owned());
  }
#endif /* UNIV_DEBUG */

  /** @return non-const version of the policy */
  MutexPolicy &policy() UNIV_NOTHROW { return (m_policy); }

  /** @return const version of the policy */
  const MutexPolicy &policy() const UNIV_NOTHROW { return (m_policy); }

 private:
  /** @return the lock state. */
  lock_word_t state() const UNIV_NOTHROW { return (m_lock_word); }

  /** Release the mutex.
  @return the new state of the mutex */
  lock_word_t unlock() UNIV_NOTHROW {
    return (TAS(&m_lock_word, MUTEX_STATE_UNLOCKED));
  }

  /** Note that there are threads waiting and need to be woken up.
  @return true if state was MUTEX_STATE_UNLOCKED (ie. granted) */
  bool set_waiters() UNIV_NOTHROW {
    return (TAS(&m_lock_word, MUTEX_STATE_WAITERS) == MUTEX_STATE_UNLOCKED);
  }

  /** Set the waiters flag, only if the mutex is locked
  @return true if succesful. */
  bool try_set_waiters() UNIV_NOTHROW {
    return (CAS(&m_lock_word, MUTEX_STATE_LOCKED, MUTEX_STATE_WAITERS) !=
            MUTEX_STATE_UNLOCKED);
  }

  /** Wait if the lock is contended.
  @return the number of waits */
  uint32_t wait() UNIV_NOTHROW {
    uint32_t n_waits = 0;

    /* Use FUTEX_WAIT_PRIVATE because our mutexes are
    not shared between processes. */

    do {
      ++n_waits;

      syscall(SYS_futex, &m_lock_word, FUTEX_WAIT_PRIVATE, MUTEX_STATE_WAITERS,
              0, 0, 0);

      // Since we are retrying the operation the return
      // value doesn't matter.

    } while (!set_waiters());

    return (n_waits);
  }

  /** Wakeup a waiting thread */
  void signal() UNIV_NOTHROW {
    syscall(SYS_futex, &m_lock_word, FUTEX_WAKE_PRIVATE, 1, 0, 0, 0);
  }

  /** Poll waiting for mutex to be unlocked.
  @param[in]	max_spins	max spins
  @param[in]	max_delay	max delay per spin
  @param[out]	n_spins		retries before acquire
  @return value of lock word before locking. */
  lock_word_t ttas(uint32_t max_spins, uint32_t max_delay,
                   uint32_t &n_spins) UNIV_NOTHROW {
    std::atomic_thread_fence(std::memory_order_acquire);

    for (n_spins = 0; n_spins < max_spins; ++n_spins) {
      if (!is_locked()) {
        lock_word_t lock = trylock();

        if (lock == MUTEX_STATE_UNLOCKED) {
          /* Lock successful */
          return (lock);
        }
      }

      ut_delay(ut_rnd_interval(0, max_delay));
    }

    return (trylock());
  }

 private:
  /** Policy data */
  MutexPolicy m_policy;

  /** lock_word is the target of the atomic test-and-set instruction
  when atomic operations are enabled. */
  alignas(ulint) lock_word_t m_lock_word;
};

#endif /* HAVE_IB_LINUX_FUTEX */

#endif
