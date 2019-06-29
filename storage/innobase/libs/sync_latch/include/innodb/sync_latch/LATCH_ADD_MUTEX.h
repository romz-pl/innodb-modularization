#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_meta.h>
#include <innodb/allocator/UT_NEW_NOKEY.h>

#ifndef UNIV_LIBRARY

#ifdef UNIV_PFS_MUTEX
/** Latch element
Used for mutexes which have PFS keys defined under UNIV_PFS_MUTEX.
@param[in]	id		Latch id
@param[in]	level		Latch level
@param[in]	key		PFS key */
#define LATCH_ADD_MUTEX(id, level, key) \
  latch_meta[LATCH_ID_##id] =           \
      UT_NEW_NOKEY(latch_meta_t(LATCH_ID_##id, #id, level, #level, key))

#else

#define LATCH_ADD_MUTEX(id, level, key) \
  latch_meta[LATCH_ID_##id] =           \
      UT_NEW_NOKEY(latch_meta_t(LATCH_ID_##id, #id, level, #level))

#endif
#endif
