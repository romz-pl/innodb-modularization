#include <innodb/sync_rw/rw_lock_remove_debug_info.h>


#ifdef UNIV_DEBUG

/** Removes a debug information struct for an rw-lock. */
void rw_lock_remove_debug_info(rw_lock_t *lock, /*!< in: rw-lock */
                               ulint pass,      /*!< in: pass value */
                               ulint lock_type) /*!< in: lock type */
{
  rw_lock_debug_t *info;

  ut_ad(lock);

  if (pass == 0 && lock_type != RW_LOCK_X_WAIT) {
    sync_check_unlock(lock);
  }

  rw_lock_debug_mutex_enter();

  for (info = UT_LIST_GET_FIRST(lock->debug_list); info != 0;
       info = UT_LIST_GET_NEXT(list, info)) {
    if (pass == info->pass &&
        (pass != 0 || os_thread_eq(info->thread_id, os_thread_get_curr_id())) &&
        info->lock_type == lock_type) {
      /* Found! */
      UT_LIST_REMOVE(lock->debug_list, info);

      rw_lock_debug_mutex_exit();

      rw_lock_debug_free(info);

      return;
    }
  }

  ut_error;
}

#endif
