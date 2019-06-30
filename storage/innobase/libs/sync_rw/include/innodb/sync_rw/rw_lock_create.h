#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_create_func.h>
#include <innodb/sync_rw/pfs_rw_lock_create_func.h>

/** Creates, or rather, initializes an rw-lock object in a specified memory
 location (which must be appropriately aligned). The rw-lock is initialized
 to the non-locked state. Explicit freeing of the rw-lock with rw_lock_free
 is necessary only if the memory block containing it is freed.
 if MySQL performance schema is enabled and "UNIV_PFS_RWLOCK" is
 defined, the rwlock are instrumented with performance schema probes. */

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_PFS_RWLOCK
    #ifdef UNIV_DEBUG

        #define rw_lock_create(K, L, level) \
          rw_lock_create_func((L), (level), #L, __FILE__, __LINE__)

    #else /* UNIV_DEBUG */

        #define rw_lock_create(K, L, level) rw_lock_create_func((L), __FILE__, __LINE__)

    #endif /* UNIV_DEBUG */

#else /* !UNIV_PFS_RWLOCK */

    /* Following macros point to Performance Schema instrumented functions. */
    #ifdef UNIV_DEBUG

        #define rw_lock_create(K, L, level) \
          pfs_rw_lock_create_func((K), (L), (level), #L, __FILE__, __LINE__)

    #else /* UNIV_DEBUG */

        #define rw_lock_create(K, L, level) \
          pfs_rw_lock_create_func((K), (L), __FILE__, __LINE__)

    #endif /* UNIV_DEBUG */

#endif /* UNIV_PFS_RWLOCK */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
