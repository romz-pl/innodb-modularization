#include <innodb/sync_array/sync_array_wait_event.h>

#include <innodb/sync_array/sync_array_enter.h>
#include <innodb/sync_event/os_event_wait_low.h>
#include <innodb/sync_array/sync_cell_get_event.h>
#include <innodb/sync_array/sync_array_free_cell.h>
#include <innodb/sync_array/sync_array_exit.h>

/** This function should be called when a thread starts to wait on
 a wait array cell. In the debug version this function checks
 if the wait for a semaphore will result in a deadlock, in which
 case prints info and asserts. */
void sync_array_wait_event(
    sync_array_t *arr,  /*!< in: wait array */
    sync_cell_t *&cell) /*!< in: index of the reserved cell */
{
  sync_array_enter(arr);

  ut_ad(!cell->waiting);
  ut_ad(cell->latch.mutex);
  ut_ad(os_thread_get_curr_id() == cell->thread_id);

  cell->waiting = true;

#ifdef UNIV_DEBUG

  /* We use simple enter to the mutex below, because if
  we cannot acquire it at once, mutex_enter would call
  recursively sync_array routines, leading to trouble.
  rw_lock_debug_mutex freezes the debug lists. */

  rw_lock_debug_mutex_enter();

  if (sync_array_detect_deadlock(arr, cell, cell, 0)) {
#ifdef UNIV_NO_ERR_MSGS
    ib::fatal()
#else
    ib::fatal(ER_IB_MSG_1157)
#endif /* UNIV_NO_ERR_MSGS */
        << "########################################"
           " Deadlock Detected!";
  }

  rw_lock_debug_mutex_exit();
#endif /* UNIV_DEBUG */
  sync_array_exit(arr);

  os_event_wait_low(sync_cell_get_event(cell), cell->signal_count);

  sync_array_free_cell(arr, cell);

  cell = 0;
}

