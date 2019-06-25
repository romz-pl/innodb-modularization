#pragma once

#include "my_config.h"

#if defined HAVE_VALGRIND
#define UNIV_DEBUG_VALGRIND
#endif /* HAVE_VALGRIND */

#ifdef UNIV_DEBUG_VALGRIND

#include <stdio.h>
#include <innodb/error/ut_error.h>

#include <valgrind/memcheck.h>

#define UNIV_MEM_VALID(addr, size) VALGRIND_MAKE_MEM_DEFINED(addr, size)
#define UNIV_MEM_INVALID(addr, size) VALGRIND_MAKE_MEM_UNDEFINED(addr, size)
#define UNIV_MEM_FREE(addr, size) VALGRIND_MAKE_MEM_NOACCESS(addr, size)
#define UNIV_MEM_ALLOC(addr, size) VALGRIND_MAKE_MEM_UNDEFINED(addr, size)
#define UNIV_MEM_DESC(addr, size) VALGRIND_CREATE_BLOCK(addr, size, #addr)
#define UNIV_MEM_UNDESC(b) VALGRIND_DISCARD(b)
#define UNIV_MEM_ASSERT_RW_LOW(addr, size, should_abort)                      \
  do {                                                                        \
    const void *_p =                                                          \
        (const void *)(ulint)VALGRIND_CHECK_MEM_IS_DEFINED(addr, size);       \
    if (UNIV_LIKELY_NULL(_p)) {                                               \
      fprintf(stderr, "%s:%d: %p[%u] undefined at %ld\n", __FILE__, __LINE__, \
              (const void *)(addr), (unsigned)(size),                         \
              (long)(((const char *)_p) - ((const char *)(addr))));           \
      if (should_abort) {                                                     \
        ut_error;                                                             \
      }                                                                       \
    }                                                                         \
  } while (0)
#define UNIV_MEM_ASSERT_RW(addr, size) UNIV_MEM_ASSERT_RW_LOW(addr, size, false)
#define UNIV_MEM_ASSERT_RW_ABORT(addr, size) \
  UNIV_MEM_ASSERT_RW_LOW(addr, size, true)
#define UNIV_MEM_ASSERT_W(addr, size)                                          \
  do {                                                                         \
    const void *_p =                                                           \
        (const void *)(ulint)VALGRIND_CHECK_MEM_IS_ADDRESSABLE(addr, size);    \
    if (UNIV_LIKELY_NULL(_p))                                                  \
      fprintf(stderr, "%s:%d: %p[%u] unwritable at %ld\n", __FILE__, __LINE__, \
              (const void *)(addr), (unsigned)(size),                          \
              (long)(((const char *)_p) - ((const char *)(addr))));            \
  } while (0)
#define UNIV_MEM_TRASH(addr, c, size) \
  do {                                \
    void *p = (addr);                 \
    ut_d(memset(p, c, size));         \
    UNIV_MEM_INVALID(addr, size);     \
  } while (0)
#else
#define UNIV_MEM_VALID(addr, size) \
  do {                             \
  } while (0)
#define UNIV_MEM_INVALID(addr, size) \
  do {                               \
  } while (0)
#define UNIV_MEM_FREE(addr, size) \
  do {                            \
  } while (0)
#define UNIV_MEM_ALLOC(addr, size) \
  do {                             \
  } while (0)
#define UNIV_MEM_DESC(addr, size) \
  do {                            \
  } while (0)
#define UNIV_MEM_UNDESC(b) \
  do {                     \
  } while (0)
#define UNIV_MEM_ASSERT_RW_LOW(addr, size, should_abort) \
  do {                                                   \
  } while (0)
#define UNIV_MEM_ASSERT_RW(addr, size) \
  do {                                 \
  } while (0)
#define UNIV_MEM_ASSERT_RW_ABORT(addr, size) \
  do {                                       \
  } while (0)
#define UNIV_MEM_ASSERT_W(addr, size) \
  do {                                \
  } while (0)
#define UNIV_MEM_TRASH(addr, c, size) \
  do {                                \
  } while (0)
#endif
#define UNIV_MEM_ASSERT_AND_FREE(addr, size) \
  do {                                       \
    UNIV_MEM_ASSERT_W(addr, size);           \
    UNIV_MEM_FREE(addr, size);               \
  } while (0)
#define UNIV_MEM_ASSERT_AND_ALLOC(addr, size) \
  do {                                        \
    UNIV_MEM_ASSERT_W(addr, size);            \
    UNIV_MEM_ALLOC(addr, size);               \
  } while (0)
