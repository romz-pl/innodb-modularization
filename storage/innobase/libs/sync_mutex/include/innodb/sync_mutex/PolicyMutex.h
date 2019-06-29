#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_latch/sync_latch_get_pfs_key.h>
#include <innodb/sync_os/sys_mutex_t.h>
#include <innodb/sync_policy/NoPolicy.h>

/** Mutex interface for all policy mutexes. This class handles the interfacing
with the Performance Schema instrumentation. */
template <typename MutexImpl>
struct PolicyMutex {
  typedef MutexImpl MutexType;
  typedef typename MutexImpl::MutexPolicy Policy;

  PolicyMutex() UNIV_NOTHROW : m_impl() {
#ifdef UNIV_PFS_MUTEX
    m_ptr = 0;
#endif /* UNIV_PFS_MUTEX */
  }

  ~PolicyMutex() {}

  /** @return non-const version of the policy */
  Policy &policy() UNIV_NOTHROW { return (m_impl.policy()); }

  /** @return const version of the policy */
  const Policy &policy() const UNIV_NOTHROW { return (m_impl.policy()); }

  /** Release the mutex. */
  void exit() UNIV_NOTHROW {
#ifdef UNIV_PFS_MUTEX
    pfs_exit();
#endif /* UNIV_PFS_MUTEX */

    policy().release(m_impl);

    m_impl.exit();
  }

  /** Acquire the mutex.
  @param n_spins	max number of spins
  @param n_delay	max delay per spin
  @param name	filename where locked
  @param line	line number where locked */
  void enter(uint32_t n_spins, uint32_t n_delay, const char *name,
             uint32_t line) UNIV_NOTHROW {
#ifdef UNIV_PFS_MUTEX
    /* Note: locker is really an alias for state. That's why
    it has to be in the same scope during pfs_end(). */

    PSI_mutex_locker_state state;
    PSI_mutex_locker *locker;

    locker = pfs_begin_lock(&state, name, line);
#endif /* UNIV_PFS_MUTEX */

    policy().enter(m_impl, name, line);

    m_impl.enter(n_spins, n_delay, name, line);

    policy().locked(m_impl, name, line);
#ifdef UNIV_PFS_MUTEX
    pfs_end(locker, 0);
#endif /* UNIV_PFS_MUTEX */
  }

  /** Try and lock the mutex, return 0 on SUCCESS and 1 otherwise.
  @param name	filename where locked
  @param line	line number where locked */
  int trylock(const char *name, uint32_t line) UNIV_NOTHROW {
#ifdef UNIV_PFS_MUTEX
    /* Note: locker is really an alias for state. That's why
    it has to be in the same scope during pfs_end(). */

    PSI_mutex_locker_state state;
    PSI_mutex_locker *locker;

    locker = pfs_begin_trylock(&state, name, line);
#endif /* UNIV_PFS_MUTEX */

    /* There is a subtlety here, we check the mutex ordering
    after locking here. This is only done to avoid add and
    then remove if the trylock was unsuccesful. */

    int ret = m_impl.try_lock() ? 0 : 1;

    if (ret == 0) {
      policy().enter(m_impl, name, line);

      policy().locked(m_impl, name, line);
    }

#ifdef UNIV_PFS_MUTEX
    pfs_end(locker, 0);
#endif /* UNIV_PFS_MUTEX */

    return (ret);
  }

#ifdef UNIV_DEBUG
  /** @return true if the thread owns the mutex. */
  bool is_owned() const UNIV_NOTHROW { return (m_impl.is_owned()); }
#endif /* UNIV_DEBUG */

  /**
  Initialise the mutex.

  @param[in]	id              Mutex ID
  @param[in]	filename	file where created
  @param[in]	line		line number in file where created */
  void init(latch_id_t id, const char *filename, uint32_t line) UNIV_NOTHROW {
#ifdef UNIV_PFS_MUTEX
    pfs_add(sync_latch_get_pfs_key(id));
#endif /* UNIV_PFS_MUTEX */

    m_impl.init(id, filename, line);
  }

  /** Free resources (if any) */
  void destroy() UNIV_NOTHROW {
#ifdef UNIV_PFS_MUTEX
    pfs_del();
#endif /* UNIV_PFS_MUTEX */
    m_impl.destroy();
  }

  /** Required for os_event_t */
  operator sys_mutex_t *() UNIV_NOTHROW {
    return (m_impl.operator sys_mutex_t *());
  }

#ifdef UNIV_PFS_MUTEX
  /** Performance schema monitoring - register mutex with PFS.

  Note: This is public only because we want to get around an issue
  with registering a subset of buffer pool pages with PFS when
  PFS_GROUP_BUFFER_SYNC is defined. Therefore this has to then
  be called by external code (see buf0buf.cc).

  @param key - Performance Schema key. */
  void pfs_add(mysql_pfs_key_t key) UNIV_NOTHROW {
    ut_ad(m_ptr == 0);
    m_ptr = PSI_MUTEX_CALL(init_mutex)(key.m_value, this);
  }

 private:
  /** Performance schema monitoring.
  @param state - PFS locker state
  @param name - file name where locked
  @param line - line number in file where locked */
  PSI_mutex_locker *pfs_begin_lock(PSI_mutex_locker_state *state,
                                   const char *name,
                                   uint32_t line) UNIV_NOTHROW {
    if (m_ptr != 0) {
      return (PSI_MUTEX_CALL(start_mutex_wait)(state, m_ptr, PSI_MUTEX_LOCK,
                                               name, (uint)line));
    }

    return (0);
  }

  /** Performance schema monitoring.
  @param state - PFS locker state
  @param name - file name where locked
  @param line - line number in file where locked */
  PSI_mutex_locker *pfs_begin_trylock(PSI_mutex_locker_state *state,
                                      const char *name,
                                      uint32_t line) UNIV_NOTHROW {
    if (m_ptr != 0) {
      return (PSI_MUTEX_CALL(start_mutex_wait)(state, m_ptr, PSI_MUTEX_TRYLOCK,
                                               name, (uint)line));
    }

    return (0);
  }

  /** Performance schema monitoring
  @param locker - PFS identifier
  @param ret - 0 for success and 1 for failure */
  void pfs_end(PSI_mutex_locker *locker, int ret) UNIV_NOTHROW {
    if (locker != 0) {
      PSI_MUTEX_CALL(end_mutex_wait)(locker, ret);
    }
  }

  /** Performance schema monitoring - register mutex release */
  void pfs_exit() {
    if (m_ptr != 0) {
      PSI_MUTEX_CALL(unlock_mutex)(m_ptr);
    }
  }

  /** Performance schema monitoring - deregister */
  void pfs_del() {
    if (m_ptr != 0) {
      PSI_MUTEX_CALL(destroy_mutex)(m_ptr);
      m_ptr = 0;
    }
  }
#endif /* UNIV_PFS_MUTEX */

 private:
  /** The mutex implementation */
  MutexImpl m_impl;

#ifdef UNIV_PFS_MUTEX
  /** The performance schema instrumentation hook. */
  PSI_mutex *m_ptr;
#endif /* UNIV_PFS_MUTEX */
};
