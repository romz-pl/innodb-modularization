#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_meta.h>
#include <innodb/allocator/UT_NEW_NOKEY.h>

#ifndef UNIV_LIBRARY
    #ifdef UNIV_PFS_MUTEX
        #ifdef UNIV_PFS_RWLOCK
            /** Latch element.
            Used for rwlocks which have PFS keys defined under UNIV_PFS_RWLOCK.
            @param[in]	id		Latch id
            @param[in]	level		Latch level
            @param[in]	key		PFS key */
            #define LATCH_ADD_RWLOCK(id, level, key) \
              latch_meta[LATCH_ID_##id] =            \
                  UT_NEW_NOKEY(latch_meta_t(LATCH_ID_##id, #id, level, #level, key))
        #else
            #define LATCH_ADD_RWLOCK(id, level, key)    \
              latch_meta[LATCH_ID_##id] = UT_NEW_NOKEY( \
                  latch_meta_t(LATCH_ID_##id, #id, level, #level, PSI_NOT_INSTRUMENTED))
        #endif /* UNIV_PFS_RWLOCK */

    #else

        #define LATCH_ADD_RWLOCK(id, level, key) \
          latch_meta[LATCH_ID_##id] =            \
              UT_NEW_NOKEY(latch_meta_t(LATCH_ID_##id, #id, level, #level))
    #endif /* UNIV_PFS_MUTEX */

#endif /* UNIV_LIBRARY */


