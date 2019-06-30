#include <innodb/sync_rw/rw_lock_list_print_info.h>

#ifdef UNIV_DEBUG

/** Prints debug info of currently locked rw-locks. */
void rw_lock_list_print_info(FILE *file) /*!< in: file where to print */
{
  ulint count = 0;

  mutex_enter(&rw_lock_list_mutex);

  fputs(
      "-------------\n"
      "RW-LATCH INFO\n"
      "-------------\n",
      file);

  for (const rw_lock_t *lock = UT_LIST_GET_FIRST(rw_lock_list); lock != NULL;
       lock = UT_LIST_GET_NEXT(list, lock)) {
    count++;

#ifndef INNODB_RW_LOCKS_USE_ATOMICS
    mutex_enter(&lock->mutex);
#endif /* INNODB_RW_LOCKS_USE_ATOMICS */

    if (lock->lock_word != X_LOCK_DECR) {
      fprintf(file, "RW-LOCK: %p ", (void *)lock);

      if (rw_lock_get_waiters(lock)) {
        fputs(" Waiters for the lock exist\n", file);
      } else {
        putc('\n', file);
      }

      rw_lock_debug_t *info;

      rw_lock_debug_mutex_enter();

      for (info = UT_LIST_GET_FIRST(lock->debug_list); info != NULL;
           info = UT_LIST_GET_NEXT(list, info)) {
        rw_lock_debug_print(file, info);
      }

      rw_lock_debug_mutex_exit();
    }

#ifndef INNODB_RW_LOCKS_USE_ATOMICS
    mutex_exit(&lock->mutex);
#endif /* INNODB_RW_LOCKS_USE_ATOMICS */
  }

  fprintf(file, "Total number of rw-locks " ULINTPF "\n", count);
  mutex_exit(&rw_lock_list_mutex);
}

#endif
