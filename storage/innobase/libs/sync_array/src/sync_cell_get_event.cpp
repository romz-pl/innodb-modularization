#include <innodb/sync_array/sync_cell_get_event.h>


/** Returns the event that the thread owning the cell waits for. */
os_event_t sync_cell_get_event(
    sync_cell_t *cell) /*!< in: non-empty sync array cell */
{
  ulint type = cell->request_type;

  if (type == SYNC_MUTEX) {
    return (cell->latch.mutex->event());

  } else if (type == SYNC_BUF_BLOCK) {
    return (cell->latch.bpmutex->event());

  } else if (type == RW_LOCK_X_WAIT) {
    return (cell->latch.lock->wait_ex_event);

  } else { /* RW_LOCK_S and RW_LOCK_X wait on the same event */

    return (cell->latch.lock->event);
  }
}
