#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_event/EventMutex.h>
#include <innodb/sync_event/os_cond_t.h>
#include <innodb/sync_event/event_iter_t.h>


/** InnoDB condition variable. */
struct os_event {
  os_event(const char *name) UNIV_NOTHROW;

  ~os_event() UNIV_NOTHROW;

  /**
  Destroys a condition variable */
  void destroy() UNIV_NOTHROW {
#ifndef _WIN32
    int ret = pthread_cond_destroy(&cond_var);
    ut_a(ret == 0);
#endif /* !_WIN32 */

    mutex.destroy();
  }

  /** Set the event */
  void set() UNIV_NOTHROW {
    mutex.enter();

    if (!m_set) {
      broadcast();
    }

    mutex.exit();
  }

  bool try_set() UNIV_NOTHROW {
    if (mutex.try_lock()) {
      if (!m_set) {
        broadcast();
      }

      mutex.exit();

      return (true);
    }

    return (false);
  }

  int64_t reset() UNIV_NOTHROW {
    mutex.enter();

    if (m_set) {
      m_set = false;
    }

    int64_t ret = signal_count;

    mutex.exit();

    return (ret);
  }

  /**
  Waits for an event object until it is in the signaled state.

  Typically, if the event has been signalled after the os_event_reset()
  we'll return immediately because event->m_set == true.
  There are, however, situations (e.g.: sync_array code) where we may
  lose this information. For example:

  thread A calls os_event_reset()
  thread B calls os_event_set()   [event->m_set == true]
  thread C calls os_event_reset() [event->m_set == false]
  thread A calls os_event_wait()  [infinite wait!]
  thread C calls os_event_wait()  [infinite wait!]

  Where such a scenario is possible, to avoid infinite wait, the
  value returned by reset() should be passed in as
  reset_sig_count. */
  void wait_low(int64_t reset_sig_count) UNIV_NOTHROW;

  /**
  Waits for an event object until it is in the signaled state or
  a timeout is exceeded.
  @param time_in_usec timeout in microseconds,
                  or OS_SYNC_INFINITE_TIME
  @param reset_sig_count zero or the value returned by
                  previous call of os_event_reset().
  @return	0 if success, OS_SYNC_TIME_EXCEEDED if timeout was exceeded */
  ulint wait_time_low(ulint time_in_usec, int64_t reset_sig_count) UNIV_NOTHROW;

  /** @return true if the event is in the signalled state. */
  bool is_set() const UNIV_NOTHROW { return (m_set); }

 private:
  /**
  Initialize a condition variable */
  void init() UNIV_NOTHROW {
    mutex.init();

#ifdef _WIN32
    InitializeConditionVariable(&cond_var);
#else
    {
      int ret;

      ret = pthread_cond_init(&cond_var, NULL);
      ut_a(ret == 0);
    }
#endif /* _WIN32 */
  }

  /**
  Wait on condition variable */
  void wait() UNIV_NOTHROW {
#ifdef _WIN32
    if (!SleepConditionVariableCS(&cond_var, mutex, INFINITE)) {
      ut_error;
    }
#else
    {
      int ret;

      ret = pthread_cond_wait(&cond_var, mutex);
      ut_a(ret == 0);
    }
#endif /* _WIN32 */
  }

  /**
  Wakes all threads waiting for condition variable */
  void broadcast() UNIV_NOTHROW {
    m_set = true;
    ++signal_count;

#ifdef _WIN32
    WakeAllConditionVariable(&cond_var);
#else
    {
      int ret;

      ret = pthread_cond_broadcast(&cond_var);
      ut_a(ret == 0);
    }
#endif /* _WIN32 */
  }

  /**
  Wakes one thread waiting for condition variable */
  void signal() UNIV_NOTHROW {
#ifdef _WIN32
    WakeConditionVariable(&cond_var);
#else
    {
      int ret;

      ret = pthread_cond_signal(&cond_var);
      ut_a(ret == 0);
    }
#endif /* _WIN32 */
  }

  /**
  Do a timed wait on condition variable.
  @return true if timed out, false otherwise */
  bool timed_wait(
#ifndef _WIN32
      const timespec *abstime /*!< Timeout. */
#else
      DWORD time_in_ms /*!< Timeout in milliseconds. */
#endif /* !_WIN32 */
  );

 private:
  bool m_set;           /*!< this is true when the
                        event is in the signaled
                        state, i.e., a thread does
                        not stop if it tries to wait
                        for this event */
  int64_t signal_count; /*!< this is incremented
                        each time the event becomes
                        signaled */
  EventMutex mutex;     /*!< this mutex protects
                        the next fields */

  os_cond_t cond_var; /*!< condition variable is
                      used in waiting for the event */

 public:
  event_iter_t event_iter; /*!< For O(1) removal from
                           list */
 protected:
  // Disable copying
  os_event(const os_event &);
  os_event &operator=(const os_event &);
};
