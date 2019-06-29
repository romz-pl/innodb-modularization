#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY

#include <innodb/sync_os/OSMutex.h>
#include <innodb/sync_policy/NoPolicy.h>


/** OS mutex for tracking lock/unlock for debugging */
template <template <typename> class Policy = NoPolicy>
struct OSTrackMutex {
  typedef Policy<OSTrackMutex> MutexPolicy;

  explicit OSTrackMutex(bool destroy_mutex_at_exit = true) UNIV_NOTHROW {
    ut_d(m_freed = true);
    ut_d(m_locked = false);
    ut_d(m_destroy_at_exit = destroy_mutex_at_exit);
  }

  ~OSTrackMutex() UNIV_NOTHROW { ut_ad(!m_destroy_at_exit || !m_locked); }

  /** Initialise the mutex.
  @param[in]	id              Mutex ID
  @param[in]	filename	File where mutex was created
  @param[in]	line		Line in filename */
  void init(latch_id_t id, const char *filename, uint32_t line) UNIV_NOTHROW {
    ut_ad(m_freed);
    ut_ad(!m_locked);

    m_mutex.init();

    ut_d(m_freed = false);

    m_policy.init(*this, id, filename, line);
  }

  /** Destroy the mutex */
  void destroy() UNIV_NOTHROW {
    ut_ad(!m_locked);
    ut_ad(innodb_calling_exit || !m_freed);

    m_mutex.destroy();

    ut_d(m_freed = true);

    m_policy.destroy();
  }

  /** Release the mutex. */
  void exit() UNIV_NOTHROW {
    ut_ad(m_locked);
    ut_d(m_locked = false);
    ut_ad(innodb_calling_exit || !m_freed);

    m_mutex.exit();
  }

  /** Acquire the mutex.
  @param[in]	max_spins	max number of spins
  @param[in]	max_delay	max delay per spin
  @param[in]	filename	from where called
  @param[in]	line		within filename */
  void enter(uint32_t max_spins, uint32_t max_delay, const char *filename,
             uint32_t line) UNIV_NOTHROW {
    ut_ad(innodb_calling_exit || !m_freed);

    m_mutex.enter();

    ut_ad(!m_locked);
    ut_d(m_locked = true);
  }

  /** @return true if locking succeeded */
  bool try_lock() UNIV_NOTHROW {
    ut_ad(innodb_calling_exit || !m_freed);

    bool locked = m_mutex.try_lock();

    if (locked) {
      ut_ad(!m_locked);
      ut_d(m_locked = locked);
    }

    return (locked);
  }

#ifdef UNIV_DEBUG
  /** @return true if the thread owns the mutex. */
  bool is_owned() const UNIV_NOTHROW {
    return (m_locked && m_policy.is_owned());
  }
#endif /* UNIV_DEBUG */

  /** @return non-const version of the policy */
  MutexPolicy &policy() UNIV_NOTHROW { return (m_policy); }

  /** @return the const version of the policy */
  const MutexPolicy &policy() const UNIV_NOTHROW { return (m_policy); }

 private:
#ifdef UNIV_DEBUG
  /** true if the mutex has not be initialized */
  bool m_freed;

  /** true if the mutex has been locked. */
  bool m_locked;

  /** Do/Dont destroy mutex at exit */
  bool m_destroy_at_exit;
#endif /* UNIV_DEBUG */

  /** OS Mutex instance */
  OSMutex m_mutex;

  /** Policy data */
  MutexPolicy m_policy;
};

#endif
