#include <innodb/sync_rw/rw_lock_free_func.h>

#include <innodb/sync_rw/rw_lock_list_mutex.h>
#include <innodb/sync_rw/rw_lock_list.h>
#include <innodb/sync_rw/X_LOCK_DECR.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>

/** Calling this function is obligatory only if the memory buffer containing
 the rw-lock is freed. Removes an rw-lock object from the global list. The
 rw-lock is checked to be in the non-locked state. */
void rw_lock_free_func(rw_lock_t *lock) /*!< in/out: rw-lock */
{
  os_rmb;
  ut_ad(rw_lock_validate(lock));
  ut_a(lock->lock_word == X_LOCK_DECR);

  mutex_enter(&rw_lock_list_mutex);

#ifndef INNODB_RW_LOCKS_USE_ATOMICS
  mutex_free(rw_lock_get_mutex(lock));
#endif /* !INNODB_RW_LOCKS_USE_ATOMICS */

  os_event_destroy(lock->event);

  os_event_destroy(lock->wait_ex_event);

  UT_LIST_REMOVE(rw_lock_list, lock);

  mutex_exit(&rw_lock_list_mutex);

  /* We did an in-place new in rw_lock_create_func() */
  ut_d(lock->~rw_lock_t());
}
