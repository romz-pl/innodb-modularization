#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY

#include <innodb/assert/assert.h>
#include <innodb/atomic/atomic.h>
#include <innodb/random/random.h>
#include <innodb/sync_mutex/mutex_state_t.h>
#include <innodb/sync_policy/NoPolicy.h>
#include <innodb/thread/os_thread_yield.h>
#include <innodb/wait/ut_delay.h>

#include <atomic>


template <template <typename> class Policy = NoPolicy>
struct TTASMutex {
  typedef Policy<TTASMutex> MutexPolicy;

  TTASMutex() UNIV_NOTHROW : m_lock_word(MUTEX_STATE_UNLOCKED) {
    /* Check that lock_word is aligned. */
    ut_ad(!((ulint)&m_lock_word % sizeof(ulint)));
  }

  ~TTASMutex() { ut_ad(m_lock_word == MUTEX_STATE_UNLOCKED); }

  /** Called when the mutex is "created". Note: Not from the constructor
  but when the mutex is initialised.
  @param[in]	id		Mutex ID
  @param[in]	filename	File where mutex was created
  @param[in]	line		Line in filename */
  void init(latch_id_t id, const char *filename, uint32_t line) UNIV_NOTHROW {
    ut_ad(m_lock_word == MUTEX_STATE_UNLOCKED);
    m_policy.init(*this, id, filename, line);
  }

  /** Destroy the mutex. */
  void destroy() UNIV_NOTHROW {
    /* The destructor can be called at shutdown. */
    ut_ad(m_lock_word == MUTEX_STATE_UNLOCKED);
    m_policy.destroy();
  }

  /**
  Try and acquire the lock using TestAndSet.
  @return	true if lock succeeded */
  bool tas_lock() UNIV_NOTHROW {
    return (TAS(&m_lock_word, MUTEX_STATE_LOCKED) == MUTEX_STATE_UNLOCKED);
  }

  /** In theory __sync_lock_release should be used to release the lock.
  Unfortunately, it does not work properly alone. The workaround is
  that more conservative __sync_lock_test_and_set is used instead. */
  void tas_unlock() UNIV_NOTHROW {
#ifdef UNIV_DEBUG
    ut_ad(state() == MUTEX_STATE_LOCKED);

    lock_word_t lock =
#endif /* UNIV_DEBUG */

        TAS(&m_lock_word, MUTEX_STATE_UNLOCKED);

    ut_ad(lock == MUTEX_STATE_LOCKED);
  }

  /** Try and lock the mutex.
  @return true on success */
  bool try_lock() UNIV_NOTHROW { return (tas_lock()); }

  /** Release the mutex. */
  void exit() UNIV_NOTHROW { tas_unlock(); }

  /** Acquire the mutex.
  @param max_spins	max number of spins
  @param max_delay	max delay per spin
  @param filename		from where called
  @param line		within filename */
  void enter(uint32_t max_spins, uint32_t max_delay, const char *filename,
             uint32_t line) UNIV_NOTHROW {
    if (!try_lock()) {
      uint32_t n_spins = ttas(max_spins, max_delay);

      /* No OS waits for spin mutexes */
      m_policy.add(n_spins, 0);
    }
  }

  /** @return the lock state. */
  lock_word_t state() const UNIV_NOTHROW { return (m_lock_word); }

  /** @return true if locked by some thread */
  bool is_locked() const UNIV_NOTHROW {
    return (m_lock_word != MUTEX_STATE_UNLOCKED);
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
  /** Spin and try to acquire the lock.
  @param[in]	max_spins	max spins
  @param[in]	max_delay	max delay per spin
  @return number spins before acquire */
  uint32_t ttas(uint32_t max_spins, uint32_t max_delay) UNIV_NOTHROW {
    uint32_t i = 0;
    const uint32_t step = max_spins;

    std::atomic_thread_fence(std::memory_order_acquire);

    do {
      while (is_locked()) {
        ut_delay(ut_rnd_interval(0, max_delay));

        ++i;

        if (i >= max_spins) {
          max_spins += step;

          os_thread_yield();

          break;
        }
      }

    } while (!try_lock());

    return (i);
  }

 private:
  // Disable copying
  TTASMutex(const TTASMutex &);
  TTASMutex &operator=(const TTASMutex &);

  /** Policy data */
  MutexPolicy m_policy;

  /** lock_word is the target of the atomic test-and-set instruction
  when atomic operations are enabled. */
  lock_word_t m_lock_word;
};

#endif
