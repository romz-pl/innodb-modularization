#include <innodb/sync_rw/rw_lock_sx_lock_func.h>

#include <innodb/sync_rw/rw_lock_sx_lock_low.h>
#include <innodb/sync_rw/rw_lock_stats.h>
#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>
#include <innodb/sync_mutex/srv_n_spin_wait_rounds.h>
#include <innodb/sync_mutex/srv_spin_wait_delay.h>
#include <innodb/sync_rw/rw_lock_set_waiter_flag.h>

/** NOTE! Use the corresponding macro, not directly this function! Lock an
 rw-lock in SX mode for the current thread. If the rw-lock is locked
 in exclusive mode, or there is an exclusive lock request waiting,
 the function spins a preset time (controlled by SYNC_SPIN_ROUNDS), waiting
 for the lock, before suspending the thread. If the same thread has an x-lock
 on the rw-lock, locking succeed, with the following exception: if pass != 0,
 only a single sx-lock may be taken on the lock. NOTE: If the same thread has
 an s-lock, locking does not succeed! */
void rw_lock_sx_lock_func(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    ulint pass,            /*!< in: pass value; != 0, if the lock will
                           be passed to another thread to unlock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line)            /*!< in: line where requested */

{
  ulint i = 0;
  sync_array_t *sync_arr;
  ulint spin_count = 0;
  uint64_t count_os_wait = 0;
  ulint spin_wait_count = 0;

  ut_ad(rw_lock_validate(lock));
  ut_ad(!rw_lock_own(lock, RW_LOCK_S));

lock_loop:

  if (rw_lock_sx_lock_low(lock, pass, file_name, line)) {
    if (count_os_wait > 0) {
      lock->count_os_wait += static_cast<uint32_t>(count_os_wait);
      rw_lock_stats.rw_sx_os_wait_count.add(count_os_wait);
    }

    rw_lock_stats.rw_sx_spin_round_count.add(spin_count);
    rw_lock_stats.rw_sx_spin_wait_count.add(spin_wait_count);

    /* Locking succeeded */
    return;

  } else {
    ++spin_wait_count;

    /* Spin waiting for the lock_word to become free */
    os_rmb;
    while (i < srv_n_spin_wait_rounds && lock->lock_word <= X_LOCK_HALF_DECR) {
      if (srv_spin_wait_delay) {
        ut_delay(ut_rnd_interval(0, srv_spin_wait_delay));
      }

      i++;
    }

    spin_count += i;

    if (i >= srv_n_spin_wait_rounds) {
      os_thread_yield();

    } else {
      goto lock_loop;
    }
  }

  sync_cell_t *cell;

  sync_arr =
      sync_array_get_and_reserve_cell(lock, RW_LOCK_SX, file_name, line, &cell);

  /* Waiters must be set before checking lock_word, to ensure signal
  is sent. This could lead to a few unnecessary wake-up signals. */
  rw_lock_set_waiter_flag(lock);

  if (rw_lock_sx_lock_low(lock, pass, file_name, line)) {
    sync_array_free_cell(sync_arr, cell);

    if (count_os_wait > 0) {
      lock->count_os_wait += static_cast<uint32_t>(count_os_wait);
      rw_lock_stats.rw_sx_os_wait_count.add(count_os_wait);
    }

    rw_lock_stats.rw_sx_spin_round_count.add(spin_count);
    rw_lock_stats.rw_sx_spin_wait_count.add(spin_wait_count);

    /* Locking succeeded */
    return;
  }

  ++count_os_wait;

  sync_array_wait_event(sync_arr, cell);

  i = 0;

  goto lock_loop;
}

