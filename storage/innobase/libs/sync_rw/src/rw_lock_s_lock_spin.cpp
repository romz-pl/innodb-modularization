#include <innodb/sync_rw/rw_lock_s_lock_spin.h>

#include <innodb/sync_rw/rw_lock_stats.h>
#include <innodb/sync_mutex/srv_n_spin_wait_rounds.h>
#include <innodb/sync_mutex/srv_spin_wait_delay.h>
#include <innodb/sync_rw/rw_lock_s_lock_low.h>
#include <innodb/sync_rw/rw_lock_set_waiter_flag.h>


/** Lock an rw-lock in shared mode for the current thread. If the rw-lock is
 locked in exclusive mode, or there is an exclusive lock request waiting,
 the function spins a preset time (controlled by srv_n_spin_wait_rounds),
 waiting for the lock, before suspending the thread. */
void rw_lock_s_lock_spin(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    ulint pass,            /*!< in: pass value; != 0, if the lock
                           will be passed to another thread to unlock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line)            /*!< in: line where requested */
{
  ulint i = 0; /* spin round count */
  sync_array_t *sync_arr;
  ulint spin_count = 0;
  uint64_t count_os_wait = 0;

  /* We reuse the thread id to index into the counter, cache
  it here for efficiency. */

  ut_ad(rw_lock_validate(lock));
  rw_lock_stats.rw_s_spin_wait_count.inc();

lock_loop:

  /* Spin waiting for the writer field to become free */
  os_rmb;
  while (i < srv_n_spin_wait_rounds && lock->lock_word <= 0) {
    if (srv_spin_wait_delay) {
      ut_delay(ut_rnd_interval(0, srv_spin_wait_delay));
    }

    i++;
  }

  if (i >= srv_n_spin_wait_rounds) {
    os_thread_yield();
  }

  ++spin_count;

  /* We try once again to obtain the lock */
  if (rw_lock_s_lock_low(lock, pass, file_name, line)) {
    if (count_os_wait > 0) {
      lock->count_os_wait += static_cast<uint32_t>(count_os_wait);
      rw_lock_stats.rw_s_os_wait_count.add(count_os_wait);
    }

    rw_lock_stats.rw_s_spin_round_count.add(spin_count);

    return; /* Success */
  } else {
    if (i < srv_n_spin_wait_rounds) {
      goto lock_loop;
    }

    ++count_os_wait;

    sync_cell_t *cell;

    sync_arr = sync_array_get_and_reserve_cell(lock, RW_LOCK_S, file_name, line,
                                               &cell);

    /* Set waiters before checking lock_word to ensure wake-up
    signal is sent. This may lead to some unnecessary signals. */
    rw_lock_set_waiter_flag(lock);

    if (rw_lock_s_lock_low(lock, pass, file_name, line)) {
      sync_array_free_cell(sync_arr, cell);

      if (count_os_wait > 0) {
        lock->count_os_wait += static_cast<uint32_t>(count_os_wait);

        rw_lock_stats.rw_s_os_wait_count.add(count_os_wait);
      }

      rw_lock_stats.rw_s_spin_round_count.add(spin_count);

      return; /* Success */
    }

      /* see comments in trx_commit_low() to
      before_trx_state_committed_in_memory explaining
      this care to invoke the following sync check.*/
#ifdef UNIV_DEBUG
    if (lock->get_level() != SYNC_DICT_OPERATION) {
      DEBUG_SYNC_C("rw_s_lock_waiting");
    }
#endif
    sync_array_wait_event(sync_arr, cell);

    i = 0;

    goto lock_loop;
  }
}
