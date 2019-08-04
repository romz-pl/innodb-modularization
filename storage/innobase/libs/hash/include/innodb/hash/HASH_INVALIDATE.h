#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_HASH_DEBUG

#define HASH_INVALIDATE(DATA, NAME) *(void **)(&DATA->NAME) = (void *)-1

#else

#define HASH_INVALIDATE(DATA, NAME) \
  do {                              \
  } while (0)

#endif
