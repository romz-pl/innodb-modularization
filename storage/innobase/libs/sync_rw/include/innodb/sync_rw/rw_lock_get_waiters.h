#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY

/** Check if there are threads waiting for the rw-lock.
 @return 1 if waiters, 0 otherwise */
UNIV_INLINE
ulint rw_lock_get_waiters(const rw_lock_t *lock); /*!< in: rw-lock */

#endif /* UNIV_LIBRARY */


#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Check if there are threads waiting for the rw-lock.
 @return 1 if waiters, 0 otherwise */
UNIV_INLINE
ulint rw_lock_get_waiters(const rw_lock_t *lock) /*!< in: rw-lock */
{
  return (lock->waiters);
}

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
