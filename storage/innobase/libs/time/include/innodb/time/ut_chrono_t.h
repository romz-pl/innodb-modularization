#pragma once

#include <innodb/univ/univ.h>

#if defined(HAVE_SYS_TIME_H) && defined(HAVE_SYS_RESOURCE_H)

#define HAVE_UT_CHRONO_T

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>

/** A "chronometer" used to clock snippets of code.
Example usage:
        ut_chrono_t	ch("this loop");
        for (;;) { ... }
        ch.show();
would print the timings of the for() loop, prefixed with "this loop:" */
class ut_chrono_t {
 public:
  /** Constructor.
  @param[in]	name	chrono's name, used when showing the values */
  ut_chrono_t(const char *name) : m_name(name), m_show_from_destructor(true) {
    reset();
  }

  /** Resets the chrono (records the current time in it). */
  void reset() {
    gettimeofday(&m_tv, NULL);

    getrusage(RUSAGE_SELF, &m_ru);
  }

  /** Shows the time elapsed and usage statistics since the last reset. */
  void show() {
    struct rusage ru_now;
    struct timeval tv_now;
    struct timeval tv_diff;

    getrusage(RUSAGE_SELF, &ru_now);

    gettimeofday(&tv_now, NULL);

#ifndef timersub
#define timersub(a, b, r)                       \
  do {                                          \
    (r)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
    (r)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((r)->tv_usec < 0) {                     \
      (r)->tv_sec--;                            \
      (r)->tv_usec += 1000000;                  \
    }                                           \
  } while (0)
#endif /* timersub */

#define CHRONO_PRINT(type, tvp)                            \
  fprintf(stderr, "%s: %s% 5ld.%06ld sec\n", m_name, type, \
          static_cast<long>((tvp)->tv_sec), static_cast<long>((tvp)->tv_usec))

    timersub(&tv_now, &m_tv, &tv_diff);
    CHRONO_PRINT("real", &tv_diff);

    timersub(&ru_now.ru_utime, &m_ru.ru_utime, &tv_diff);
    CHRONO_PRINT("user", &tv_diff);

    timersub(&ru_now.ru_stime, &m_ru.ru_stime, &tv_diff);
    CHRONO_PRINT("sys ", &tv_diff);
  }

  /** Cause the timings not to be printed from the destructor. */
  void end() { m_show_from_destructor = false; }

  /** Destructor. */
  ~ut_chrono_t() {
    if (m_show_from_destructor) {
      show();
    }
  }

 private:
  /** Name of this chronometer. */
  const char *m_name;

  /** True if the current timings should be printed by the destructor. */
  bool m_show_from_destructor;

  /** getrusage() result as of the last reset(). */
  struct rusage m_ru;

  /** gettimeofday() result as of the last reset(). */
  struct timeval m_tv;
};

#endif /* HAVE_SYS_TIME_H && HAVE_SYS_RESOURCE_H */
