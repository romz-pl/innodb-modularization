#include <innodb/sync_array/sync_array_wake_threads_if_sema_free_low.h>

#include <innodb/sync_array/sync_array_enter.h>
#include <innodb/sync_array/sync_array_get_nth_cell.h>
#include <innodb/sync_array/sync_arr_cell_can_wake_up.h>
#include <innodb/sync_array/sync_cell_get_event.h>
#include <innodb/sync_array/sync_array_exit.h>

/** If the wakeup algorithm does not work perfectly at semaphore relases,
 this function will do the waking (see the comment in mutex_exit). This
 function should be called about every 1 second in the server.

 Note that there's a race condition between this thread and mutex_exit
 changing the lock_word and calling signal_object, so sometimes this finds
 threads to wake up even when nothing has gone wrong. */
void sync_array_wake_threads_if_sema_free_low(
    sync_array_t *arr) /* in/out: wait array */
{
  sync_array_enter(arr);

  for (ulint i = 0; i < arr->next_free_slot; ++i) {
    sync_cell_t *cell;

    cell = sync_array_get_nth_cell(arr, i);

    if (cell->latch.mutex != 0 && sync_arr_cell_can_wake_up(cell)) {
      os_event_t event;

      event = sync_cell_get_event(cell);

      os_event_set(event);
    }
  }

  sync_array_exit(arr);
}

