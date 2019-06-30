#include <innodb/sync_rw/rw_lock_get_debug_info.h>

#ifdef UNIV_DEBUG

/** Get the thread debug info
@param[in]	infos		The rw-lock mode owned by the threads
@param[in]	lock		rw-lock to check
@return the thread debug info or NULL if not found */
static void rw_lock_get_debug_info(const rw_lock_t *lock, Infos *infos) {
  rw_lock_debug_t *info = NULL;

  ut_ad(rw_lock_validate(lock));

  rw_lock_debug_mutex_enter();

  for (info = UT_LIST_GET_FIRST(lock->debug_list); info != NULL;
       info = UT_LIST_GET_NEXT(list, info)) {
    if (os_thread_eq(info->thread_id, os_thread_get_curr_id())) {
      infos->push_back(info);
    }
  }

  rw_lock_debug_mutex_exit();
}

#endif
