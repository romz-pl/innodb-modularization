#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_LIBRARY

#define rw_lock_own_flagged(A, B) true

#endif /* UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */


#ifndef UNIV_LIBRARY

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

/** Checks if the thread has locked the rw-lock in the specified mode, with
 the pass value == 0. */
bool rw_lock_own_flagged(
    const rw_lock_t *lock, /*!< in: rw-lock */
    rw_lock_flags_t flags) /*!< in: specify lock types with
                           OR of the rw_lock_flag_t values */
    MY_ATTRIBUTE((warn_unused_result));

#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
