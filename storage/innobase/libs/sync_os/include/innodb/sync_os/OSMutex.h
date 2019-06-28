#pragma once

#include <innodb/univ/univ.h>
#include <innodb/assert/assert.h>
#include <innodb/sync_os/sys_mutex_t.h>
#include <innodb/logger/error.h>

/** OS mutex, without any policy. It is a thin wrapper around the
system mutexes. The interface is different from the policy mutexes,
to ensure that it is called directly and not confused with the
policy mutexes. */
struct OSMutex {
  /** Constructor */
  OSMutex() UNIV_NOTHROW { ut_d(m_freed = true); }

  /** Create the mutex by calling the system functions. */
  void init() UNIV_NOTHROW {
    ut_ad(m_freed);

#ifdef _WIN32
    InitializeCriticalSection((LPCRITICAL_SECTION)&m_mutex);
#else
    {
      int ret = pthread_mutex_init(&m_mutex, NULL);
      ut_a(ret == 0);
    }
#endif /* _WIN32 */

    ut_d(m_freed = false);
  }

  /** Destructor */
  ~OSMutex() {}

  /** Destroy the mutex */
  void destroy() UNIV_NOTHROW {
    ut_ad(innodb_calling_exit || !m_freed);
#ifdef _WIN32
    DeleteCriticalSection((LPCRITICAL_SECTION)&m_mutex);
#else
    int ret;

    ret = pthread_mutex_destroy(&m_mutex);

    if (ret != 0) {
      ib::error() << "Return value " << ret
                  << " when calling pthread_mutex_destroy().";
    }
#endif /* _WIN32 */
    ut_d(m_freed = true);
  }

  /** Release the mutex. */
  void exit() UNIV_NOTHROW {
    ut_ad(innodb_calling_exit || !m_freed);
#ifdef _WIN32
    LeaveCriticalSection(&m_mutex);
#else
    int ret = pthread_mutex_unlock(&m_mutex);
    ut_a(ret == 0);
#endif /* _WIN32 */
  }

  /** Acquire the mutex. */
  void enter() UNIV_NOTHROW {
    ut_ad(innodb_calling_exit || !m_freed);
#ifdef _WIN32
    EnterCriticalSection((LPCRITICAL_SECTION)&m_mutex);
#else
    int ret = pthread_mutex_lock(&m_mutex);
    ut_a(ret == 0);
#endif /* _WIN32 */
  }

  /** @return true if locking succeeded */
  bool try_lock() UNIV_NOTHROW {
    ut_ad(innodb_calling_exit || !m_freed);
#ifdef _WIN32
    return (TryEnterCriticalSection(&m_mutex) != 0);
#else
    return (pthread_mutex_trylock(&m_mutex) == 0);
#endif /* _WIN32 */
  }

  /** Required for os_event_t */
  operator sys_mutex_t *() UNIV_NOTHROW { return (&m_mutex); }

#if defined(UNIV_LIBRARY) && defined(UNIV_DEBUG)
  bool is_owned() {
    /* This should never be reached. This is
    added to fix is_owned() compilation errors
    for library. We will never reach here because
    mutexes are disabled in library. */
    ut_error;
    return (false);
  }
#endif /* UNIV_LIBRARY && UNIV_DEBUG */

 private:
#ifdef UNIV_DEBUG
  /** true if the mutex has been freed/destroyed. */
  bool m_freed;
#endif /* UNIV_DEBUG */

  sys_mutex_t m_mutex;
};
