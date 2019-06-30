#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY
#ifdef UNIV_DEBUG

    /**
    Pass-through version of rw_lock_own(), which normally checks that the
    thread has locked the rw-lock in the specified mode.
    @param[in]	rw-lock		pointer to rw-lock
    @param[in]	lock type	lock type: RW_LOCK_S, RW_LOCK_X
    @return true if success */
    UNIV_INLINE
    bool rw_lock_own(rw_lock_t *lock, ulint lock_type) { return (lock != nullptr); }

#endif /* UNIV_DEBUG */
#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

/** Checks if the thread has locked the rw-lock in the specified mode, with
 the pass value == 0. */
ibool rw_lock_own(rw_lock_t *lock, /*!< in: rw-lock */
                  ulint lock_type) /*!< in: lock type: RW_LOCK_S,
                                   RW_LOCK_X */
    MY_ATTRIBUTE((warn_unused_result));

#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
