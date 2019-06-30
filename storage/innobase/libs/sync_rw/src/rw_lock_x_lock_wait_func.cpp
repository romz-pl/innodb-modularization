#include <innodb/sync_rw/rw_lock_x_lock_wait_func.h>

#include <innodb/sync_mutex/srv_spin_wait_delay.h>
#include <innodb/sync_mutex/srv_n_spin_wait_rounds.h>
#include <innodb/sync_rw/rw_lock_stats.h>

/** Function for the next writer to call. Waits for readers to exit.
 The caller must have already decremented lock_word by X_LOCK_DECR. */
void rw_lock_x_lock_wait_func(
    rw_lock_t *lock, /*!< in: pointer to rw-lock */
#ifdef UNIV_DEBUG
    ulint pass, /*!< in: pass value; != 0, if the lock will
                be passed to another thread to unlock */
#endif
    lint threshold,        /*!< in: threshold to wait for */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line)            /*!< in: line where requested */
{
  ulint i = 0;
  ulint n_spins = 0;
  sync_array_t *sync_arr;
  uint64_t count_os_wait = 0;

  os_rmb;
  ut_ad(lock->lock_word <= threshold);

  while (lock->lock_word < threshold) {
    if (srv_spin_wait_delay) {
      ut_delay(ut_rnd_interval(0, srv_spin_wait_delay));
    }

    if (i < srv_n_spin_wait_rounds) {
      i++;
      os_rmb;
      continue;
    }

    /* If there is still a reader, then go to sleep.*/
    ++n_spins;

    sync_cell_t *cell;

    sync_arr = sync_array_get_and_reserve_cell(lock, RW_LOCK_X_WAIT, file_name,
                                               line, &cell);

    i = 0;

    /* Check lock_word to ensure wake-up isn't missed.*/
    if (lock->lock_word < threshold) {
      ++count_os_wait;

      /* Add debug info as it is needed to detect possible
      deadlock. We must add info for WAIT_EX thread for
      deadlock detection to work properly. */
      ut_d(rw_lock_add_debug_info(lock, pass, RW_LOCK_X_WAIT, file_name, line));

      sync_array_wait_event(sync_arr, cell);

      ut_d(rw_lock_remove_debug_info(lock, pass, RW_LOCK_X_WAIT));

      /* It is possible to wake when lock_word < 0.
      We must pass the while-loop check to proceed.*/

    } else {
      sync_array_free_cell(sync_arr, cell);
      break;
    }
  }

  rw_lock_stats.rw_x_spin_round_count.add(n_spins);

  if (count_os_wait > 0) {
    lock->count_os_wait += static_cast<uint32_t>(count_os_wait);
    rw_lock_stats.rw_x_os_wait_count.add(count_os_wait);
  }
}
