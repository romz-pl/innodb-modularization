#include <innodb/sync_rw/rw_lock_own.h>


#ifdef UNIV_DEBUG

/** Checks if the thread has locked the rw-lock in the specified mode, with
 the pass value == 0.
 @return true if locked */
ibool rw_lock_own(rw_lock_t *lock, /*!< in: rw-lock */
                  ulint lock_type) /*!< in: lock type: RW_LOCK_S,
                                   RW_LOCK_X */
{
  ut_ad(lock);
  ut_ad(rw_lock_validate(lock));

  rw_lock_debug_mutex_enter();

  for (const rw_lock_debug_t *info = UT_LIST_GET_FIRST(lock->debug_list);
       info != NULL; info = UT_LIST_GET_NEXT(list, info)) {
    if (os_thread_eq(info->thread_id, os_thread_get_curr_id()) &&
        info->pass == 0 && info->lock_type == lock_type) {
      rw_lock_debug_mutex_exit();
      /* Found! */

      return (TRUE);
    }
  }
  rw_lock_debug_mutex_exit();

  return (FALSE);
}

#endif
