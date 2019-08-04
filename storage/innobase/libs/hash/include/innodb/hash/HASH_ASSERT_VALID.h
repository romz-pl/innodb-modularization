#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_HASH_DEBUG

#include <innodb/assert/assert.h>

#define HASH_ASSERT_VALID(DATA) ut_a((void *)(DATA) != (void *)-1)

#else

#define HASH_ASSERT_VALID(DATA) \
  do {                          \
  } while (0)

#endif
