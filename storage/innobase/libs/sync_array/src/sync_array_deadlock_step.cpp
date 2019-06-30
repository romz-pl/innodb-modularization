#include <innodb/sync_array/sync_array_deadlock_step.h>

#ifdef UNIV_DEBUG

/** Recursion step for deadlock detection.
 @return true if deadlock detected */
ibool sync_array_deadlock_step(
    sync_array_t *arr,     /*!< in: wait array; NOTE! the caller must
                           own the mutex to array */
    sync_cell_t *start,    /*!< in: cell where recursive search
                           started */
    os_thread_id_t thread, /*!< in: thread to look at */
    ulint pass,            /*!< in: pass value */
    ulint depth)           /*!< in: recursion depth */
{
  sync_cell_t *new_cell;

  if (pass != 0) {
    /* If pass != 0, then we do not know which threads are
    responsible of releasing the lock, and no deadlock can
    be detected. */

    return (FALSE);
  }

  new_cell = sync_array_find_thread(arr, thread);

  if (new_cell == start) {
    /* Deadlock */
    fputs(
        "########################################\n"
        "DEADLOCK of threads detected!\n",
        stderr);

    return (TRUE);

  } else if (new_cell) {
    return (sync_array_detect_deadlock(arr, start, new_cell, depth + 1));
  }
  return (FALSE);
}

#endif
