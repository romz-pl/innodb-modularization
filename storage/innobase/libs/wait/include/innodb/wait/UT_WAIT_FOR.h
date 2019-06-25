#pragma once

#include <innodb/univ/univ.h>
#include <innodb/thread/os_thread_sleep.h>
#include <innodb/time/ut_time_us.h>

#ifndef UNIV_HOTBACKUP

/** Delays execution for at most max_wait_us microseconds or returns earlier
 if cond becomes true.
 @param cond in: condition to wait for; evaluated every 2 ms
 @param max_wait_us in: maximum delay to wait, in microseconds */
#define UT_WAIT_FOR(cond, max_wait_us)                               \
  do {                                                               \
    uintmax_t start_us;                                              \
    start_us = ut_time_us(NULL);                                     \
    while (!(cond) && ut_time_us(NULL) - start_us < (max_wait_us)) { \
      os_thread_sleep(2000 /* 2 ms */);                              \
    }                                                                \
  } while (0)

#endif                 /* !UNIV_HOTBACKUP */
