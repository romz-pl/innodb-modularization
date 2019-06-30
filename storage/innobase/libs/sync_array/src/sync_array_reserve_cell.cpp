#include <innodb/sync_array/sync_array_reserve_cell.h>

#include <innodb/sync_array/sync_array_enter.h>
#include <innodb/sync_array/sync_array_get_nth_cell.h>
#include <innodb/sync_array/sync_array_exit.h>
#include <innodb/thread/os_thread_get_curr_id.h>
#include <innodb/time/ut_time.h>
#include <innodb/sync_array/sync_cell_get_event.h>
#include <innodb/sync_event/os_event_reset.h>


/** Reserves a wait array cell for waiting for an object.
 The event of the cell is reset to nonsignalled state.
 @return sync cell to wait on */
sync_cell_t *sync_array_reserve_cell(
    sync_array_t *arr, /*!< in: wait array */
    void *object,      /*!< in: pointer to the object to wait for */
    ulint type,        /*!< in: lock request type */
    const char *file,  /*!< in: file where requested */
    ulint line)        /*!< in: line where requested */
{
  sync_cell_t *cell;

  sync_array_enter(arr);

  if (arr->first_free_slot != ULINT_UNDEFINED) {
    /* Try and find a slot in the free list */
    ut_ad(arr->first_free_slot < arr->next_free_slot);
    cell = sync_array_get_nth_cell(arr, arr->first_free_slot);
    arr->first_free_slot = cell->line;
  } else if (arr->next_free_slot < arr->n_cells) {
    /* Try and find a slot after the currently allocated slots */
    cell = sync_array_get_nth_cell(arr, arr->next_free_slot);
    ++arr->next_free_slot;
  } else {
    sync_array_exit(arr);

    // We should return NULL and if there is more than
    // one sync array, try another sync array instance.
    return (NULL);
  }

  ++arr->res_count;

  ut_ad(arr->n_reserved < arr->n_cells);
  ut_ad(arr->next_free_slot <= arr->n_cells);

  ++arr->n_reserved;

  /* Reserve the cell. */
  ut_ad(cell->latch.mutex == NULL);

  cell->request_type = type;

  if (cell->request_type == SYNC_MUTEX) {
    cell->latch.mutex = reinterpret_cast<WaitMutex *>(object);
  } else if (cell->request_type == SYNC_BUF_BLOCK) {
    cell->latch.bpmutex = reinterpret_cast<BlockWaitMutex *>(object);
  } else {
    cell->latch.lock = reinterpret_cast<rw_lock_t *>(object);
  }

  cell->waiting = false;

  cell->file = file;
  cell->line = line;

  sync_array_exit(arr);

  cell->thread_id = os_thread_get_curr_id();

  cell->reservation_time = ut_time();

  /* Make sure the event is reset and also store the value of
  signal_count at which the event was reset. */
  os_event_t event = sync_cell_get_event(cell);
  cell->signal_count = os_event_reset(event);

  return (cell);
}
