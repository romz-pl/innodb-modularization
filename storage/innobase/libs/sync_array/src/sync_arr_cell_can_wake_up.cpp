#include <innodb/sync_array/sync_arr_cell_can_wake_up.h>

#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>


/** Determines if we can wake up the thread waiting for a sempahore. */
bool sync_arr_cell_can_wake_up(
    sync_cell_t *cell) /*!< in: cell to search */
{
  rw_lock_t *lock;

  switch (cell->request_type) {
    WaitMutex *mutex;
    BlockWaitMutex *bpmutex;
    case SYNC_MUTEX:
      mutex = cell->latch.mutex;

      os_rmb;
      if (mutex->state() == MUTEX_STATE_UNLOCKED) {
        return (true);
      }

      break;

    case SYNC_BUF_BLOCK:
      bpmutex = cell->latch.bpmutex;

      os_rmb;
      if (bpmutex->state() == MUTEX_STATE_UNLOCKED) {
        return (true);
      }

      break;

    case RW_LOCK_X:
    case RW_LOCK_SX:
      lock = cell->latch.lock;

      os_rmb;
      if (lock->lock_word > X_LOCK_HALF_DECR) {
        /* Either unlocked or only read locked. */

        return (true);
      }

      break;

    case RW_LOCK_X_WAIT:

      lock = cell->latch.lock;

      /* lock_word == 0 means all readers or sx have left */
      os_rmb;
      if (lock->lock_word == 0) {
        return (true);
      }
      break;

    case RW_LOCK_S:

      lock = cell->latch.lock;

      /* lock_word > 0 means no writer or reserved writer */
      os_rmb;
      if (lock->lock_word > 0) {
        return (true);
      }
  }

  return (false);
}

