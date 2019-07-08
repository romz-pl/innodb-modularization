#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP



/** Following are macro defines for basic montior counter manipulations.
Please note we do not provide any synchronization for these monitor
operations due to performance consideration. Most counters can
be placed under existing mutex protections in respective code
module. */

/** Macros to access various fields of a monitor counters */

#ifdef UNIV_DEBUG_VALGRIND

#include <innodb/memory_check/memory_check.h>

#define MONITOR_CHECK_DEFINED(value)  \
  do {                                \
    mon_type_t m = value;             \
    UNIV_MEM_ASSERT_RW(&m, sizeof m); \
  } while (0)

#else /* UNIV_DEBUG_VALGRIND */

#define MONITOR_CHECK_DEFINED(value) (void)0

#endif /* UNIV_DEBUG_VALGRIND */


#endif
