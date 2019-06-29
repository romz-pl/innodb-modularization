#include <innodb/sync_event/os_event.h>

#include <innodb/error/ut_error.h>
#include <innodb/sync_event/OS_SYNC_INFINITE_TIME.h>
#include <innodb/sync_event/OS_SYNC_TIME_EXCEEDED.h>
#include <innodb/time/ut_usectime.h>

/** The number of microsecnds in a second. */
static const ulint MICROSECS_IN_A_SECOND = 1000000;

bool os_event::timed_wait(
#ifndef _WIN32
    const timespec *abstime
#else
    DWORD time_in_ms
#endif /* !_WIN32 */
) {
#ifdef _WIN32
  BOOL ret;

  ret = SleepConditionVariableCS(&cond_var, mutex, time_in_ms);

  if (!ret) {
    DWORD err = GetLastError();

    /* FQDN=msdn.microsoft.com
    @see http://$FQDN/en-us/library/ms686301%28VS.85%29.aspx,

    "Condition variables are subject to spurious wakeups
    (those not associated with an explicit wake) and stolen wakeups
    (another thread manages to run before the woken thread)."
    Check for both types of timeouts.
    Conditions are checked by the caller.*/
    if (err == WAIT_TIMEOUT || err == ERROR_TIMEOUT) {
      return (true);
    }
  }

  ut_a(ret);

  return (false);
#else
  int ret;

  ret = pthread_cond_timedwait(&cond_var, mutex, abstime);

  switch (ret) {
    case 0:
    case ETIMEDOUT:
      /* We play it safe by checking for EINTR even though
      according to the POSIX documentation it can't return EINTR. */
    case EINTR:
      break;

    default:
#ifdef UNIV_NO_ERR_MSGS
      ib::error()
#else
      ib::error(ER_IB_MSG_742)
#endif /* !UNIV_NO_ERR_MSGS */
          << "pthread_cond_timedwait() returned: " << ret << ": abstime={"
          << abstime->tv_sec << "," << abstime->tv_nsec << "}";
      ut_error;
  }

  return (ret == ETIMEDOUT);
#endif /* _WIN32 */
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
void os_event::wait_low(int64_t reset_sig_count) UNIV_NOTHROW {
  mutex.enter();

  if (!reset_sig_count) {
    reset_sig_count = signal_count;
  }

  while (!m_set && signal_count == reset_sig_count) {
    wait();

    /* Spurious wakeups may occur: we have to check if the
    event really has been signaled after we came here to wait. */
  }

  mutex.exit();
}


/**
Waits for an event object until it is in the signaled state or
a timeout is exceeded.
@param time_in_usec - timeout in microseconds, or OS_SYNC_INFINITE_TIME
@param reset_sig_count - zero or the value returned by previous call
        of os_event_reset().
@return	0 if success, OS_SYNC_TIME_EXCEEDED if timeout was exceeded */
ulint os_event::wait_time_low(ulint time_in_usec,
                              int64_t reset_sig_count) UNIV_NOTHROW {
  bool timed_out = false;

#ifdef _WIN32
  DWORD time_in_ms;

  if (time_in_usec != OS_SYNC_INFINITE_TIME) {
    time_in_ms = DWORD(time_in_usec / 1000);
  } else {
    time_in_ms = INFINITE;
  }
#else
  struct timespec abstime;

  if (time_in_usec != OS_SYNC_INFINITE_TIME) {
    struct timeval tv;
    int ret;
    ulint sec;
    ulint usec;

    ret = ut_usectime(&sec, &usec);
    ut_a(ret == 0);

    tv.tv_sec = sec;
    tv.tv_usec = usec;

    tv.tv_usec += time_in_usec;

    if ((ulint)tv.tv_usec >= MICROSECS_IN_A_SECOND) {
      tv.tv_sec += tv.tv_usec / MICROSECS_IN_A_SECOND;
      tv.tv_usec %= MICROSECS_IN_A_SECOND;
    }

    abstime.tv_sec = tv.tv_sec;
    abstime.tv_nsec = tv.tv_usec * 1000;
  } else {
    abstime.tv_nsec = 999999999;
    abstime.tv_sec = std::numeric_limits<time_t>::max();
  }

  ut_a(abstime.tv_nsec <= 999999999);

#endif /* _WIN32 */

  mutex.enter();

  if (!reset_sig_count) {
    reset_sig_count = signal_count;
  }

  do {
    if (m_set || signal_count != reset_sig_count) {
      break;
    }

#ifndef _WIN32
    timed_out = timed_wait(&abstime);
#else
    timed_out = timed_wait(time_in_ms);
#endif /* !_WIN32 */

  } while (!timed_out);

  mutex.exit();

  return (timed_out ? OS_SYNC_TIME_EXCEEDED : 0);
}

/** Constructor */
os_event::os_event(const char *name) UNIV_NOTHROW {
  init();

  m_set = false;

  /* We return this value in os_event_reset(),
  which can then be be used to pass to the
  os_event_wait_low(). The value of zero is
  reserved in os_event_wait_low() for the case
  when the caller does not want to pass any
  signal_count value. To distinguish between
  the two cases we initialize signal_count
  to 1 here. */

  signal_count = 1;
}

/** Destructor */
os_event::~os_event() UNIV_NOTHROW {
    destroy();
}
