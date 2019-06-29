#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY

#include <innodb/assert/assert.h>
#include <innodb/atomic/atomic.h>
#include <innodb/random/random.h>
#include <innodb/sync_event/os_event_t.h>
#include <innodb/sync_event/os_event_create.h>
#include <innodb/sync_event/os_event_destroy.h>
#include <innodb/sync_latch/sync_latch_get_name.h>
#include <innodb/sync_mutex/mutex_state_t.h>
#include <innodb/sync_policy/NoPolicy.h>
#include <innodb/thread/os_thread_yield.h>
#include <innodb/wait/ut_delay.h>

#include <atomic>

template <template <typename> class Policy = NoPolicy>
struct TTASEventMutex {
  typedef Policy<TTASEventMutex> MutexPolicy;

  TTASEventMutex() UNIV_NOTHROW : m_lock_word(MUTEX_STATE_UNLOCKED),
                                  m_waiters(),
                                  m_event() {
    /* Check that lock_word is aligned. */
    ut_ad(!((ulint)&m_lock_word % sizeof(ulint)));
  }

  ~TTASEventMutex() UNIV_NOTHROW { ut_ad(!m_lock_word.load()); }

  /** Called when the mutex is "created". Note: Not from the constructor
  but when the mutex is initialised.
  @param[in]	id		Mutex ID
  @param[in]	filename	File where mutex was created
  @param[in]	line		Line in filename */
  void init(latch_id_t id, const char *filename, uint32_t line) UNIV_NOTHROW {
    ut_a(m_event == nullptr);
    ut_a(!m_lock_word.load(std::memory_order_relaxed));

    m_event = os_event_create(sync_latch_get_name(id));

    m_policy.init(*this, id, filename, line);
  }

  /** This is the real desctructor. This mutex can be created in BSS and
  its desctructor will be called on exit(). We can't call
  os_event_destroy() at that stage. */
  void destroy() UNIV_NOTHROW {
    ut_ad(!m_lock_word.load(std::memory_order_relaxed));

    /* We have to free the event before InnoDB shuts down. */
    os_event_destroy(m_event);
    m_event = 0;

    m_policy.destroy();
  }

  /** Try and lock the mutex. Note: POSIX returns 0 on success.
  @return true on success */
  bool try_lock() UNIV_NOTHROW {
    bool expected = false;
    return (m_lock_word.compare_exchange_strong(expected, true));
  }

  /** Release the mutex. */
  void exit() UNIV_NOTHROW {
    m_lock_word.store(false);
    std::atomic_thread_fence(std::memory_order_acquire);

    if (m_waiters.load(std::memory_order_acquire)) {
      signal();
    }
  }

  /** Acquire the mutex.
  @param[in]	max_spins	max number of spins
  @param[in]	max_delay	max delay per spin
  @param[in]	filename	from where called
  @param[in]	line		within filename */
  void enter(uint32_t max_spins, uint32_t max_delay, const char *filename,
             uint32_t line) UNIV_NOTHROW {
    if (!try_lock()) {
      spin_and_try_lock(max_spins, max_delay, filename, line);
    }
  }

  /** @return true if locked. */
  bool state() const UNIV_NOTHROW {
    return (m_lock_word.load(std::memory_order_relaxed));
  }

  /** The event that the mutex will wait in sync0arr.cc
  @return even instance */
  os_event_t event() UNIV_NOTHROW { return (m_event); }

  /** @return true if locked by some thread */
  bool is_locked() const UNIV_NOTHROW {
    return (m_lock_word.load(std::memory_order_relaxed));
  }

#ifdef UNIV_DEBUG
  /** @return true if the calling thread owns the mutex. */
  bool is_owned() const UNIV_NOTHROW {
    return (is_locked() && m_policy.is_owned());
  }
#endif /* UNIV_DEBUG */

  /** @return non-const version of the policy */
  MutexPolicy &policy() UNIV_NOTHROW { return (m_policy); }

  /** @return const version of the policy */
  const MutexPolicy &policy() const UNIV_NOTHROW { return (m_policy); }

 private:
  /** Wait in the sync array.
  @param[in]	filename	from where it was called
  @param[in]	line		line number in file
  @param[in]	spin		retry this many times again
  @return true if the mutex acquisition was successful. */
  bool wait(const char *filename, uint32_t line, uint32_t spin) UNIV_NOTHROW;

  /** Spin and wait for the mutex to become free.
  @param[in]	max_spins	max spins
  @param[in]	max_delay	max delay per spin
  @param[in,out]	n_spins		spin start index
  @return true if unlocked */
  bool is_free(uint32_t max_spins, uint32_t max_delay,
               uint32_t &n_spins) const UNIV_NOTHROW {
    ut_ad(n_spins <= max_spins);

    /* Spin waiting for the lock word to become zero. Note
    that we do not have to assume that the read access to
    the lock word is atomic, as the actual locking is always
    committed with atomic test-and-set. In reality, however,
    all processors probably have an atomic read of a memory word. */

    do {
      if (!is_locked()) {
        return (true);
      }

      ut_delay(ut_rnd_interval(0, max_delay));

      ++n_spins;

    } while (n_spins < max_spins);

    return (false);
  }

  /** Spin while trying to acquire the mutex
  @param[in]	max_spins	max number of spins
  @param[in]	max_delay	max delay per spin
  @param[in]	filename	from where called
  @param[in]	line		within filename */
  void spin_and_try_lock(uint32_t max_spins, uint32_t max_delay,
                         const char *filename, uint32_t line) UNIV_NOTHROW {
    uint32_t n_spins = 0;
    uint32_t n_waits = 0;
    const uint32_t step = max_spins;

    for (;;) {
      /* If the lock was free then try and acquire it. */

      if (is_free(max_spins, max_delay, n_spins)) {
        if (try_lock()) {
          break;
        } else {
          continue;
        }

      } else {
        max_spins = n_spins + step;
      }

      ++n_waits;

      os_thread_yield();

      /* The 4 below is a heuristic that has existed for a
      very long time now. It is unclear if changing this
      value will make a difference.

      NOTE: There is a delay that happens before the retry,
      finding a free slot in the sync arary and the yield
      above. Otherwise we could have simply done the extra
      spin above. */

      if (wait(filename, line, 4)) {
        n_spins += 4;

        break;
      }
    }

    /* Waits and yields will be the same number in our
    mutex design */

    m_policy.add(n_spins, n_waits);
  }

  /** @return the value of the m_waiters flag */
  lock_word_t waiters() UNIV_NOTHROW {
    return (m_waiters.load(std::memory_order_relaxed));
  }

  /** Note that there are threads waiting on the mutex */
  void set_waiters() UNIV_NOTHROW {
    m_waiters.store(true, std::memory_order_release);
  }

  /** Note that there are no threads waiting on the mutex */
  void clear_waiters() UNIV_NOTHROW {
    m_waiters.store(false, std::memory_order_release);
  }

  /** Wakeup any waiting thread(s). */
  void signal() UNIV_NOTHROW;

 private:
  /** Disable copying */
  TTASEventMutex(TTASEventMutex &&) = delete;
  TTASEventMutex(const TTASEventMutex &) = delete;
  TTASEventMutex &operator=(TTASEventMutex &&) = delete;
  TTASEventMutex &operator=(const TTASEventMutex &) = delete;

  /** lock_word is the target of the atomic test-and-set instruction
  when atomic operations are enabled. */
  std::atomic_bool m_lock_word;

  /** true if there are (or may be) threads waiting
  in the global wait array for this mutex to be released. */
  std::atomic_bool m_waiters;

  /** Used by sync0arr.cc for the wait queue */
  os_event_t m_event;

  /** Policy data */
  MutexPolicy m_policy;
};

#endif
