#include <innodb/sync_rw/rw_lock_add_debug_info.h>

#ifdef UNIV_DEBUG

/** Inserts the debug information for an rw-lock. */
void rw_lock_add_debug_info(
    rw_lock_t *lock,       /*!< in: rw-lock */
    ulint pass,            /*!< in: pass value */
    ulint lock_type,       /*!< in: lock type */
    const char *file_name, /*!< in: file where requested */
    ulint line)            /*!< in: line where requested */
{
  ut_ad(file_name != NULL);

  rw_lock_debug_t *info = rw_lock_debug_create();

  rw_lock_debug_mutex_enter();

  info->pass = pass;
  info->line = line;
  info->lock_type = lock_type;
  info->file_name = file_name;
  info->thread_id = os_thread_get_curr_id();

  UT_LIST_ADD_FIRST(lock->debug_list, info);

  rw_lock_debug_mutex_exit();

  if (pass == 0 && lock_type != RW_LOCK_X_WAIT) {
    /* Recursive x while holding SX
    (lock_type == RW_LOCK_X && lock_word == -X_LOCK_HALF_DECR)
    is treated as not-relock (new lock). */

    if ((lock_type == RW_LOCK_X && lock->lock_word < -X_LOCK_HALF_DECR) ||
        (lock_type == RW_LOCK_SX &&
         (lock->lock_word < 0 || lock->sx_recursive == 1))) {
      sync_check_lock_validate(lock);
      sync_check_lock_granted(lock);
    } else {
      sync_check_relock(lock);
    }
  }
}

#endif
