#include <innodb/sync_rw/rw_lock_x_lock_low.h>

#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>
#include <innodb/sync_rw/rw_lock_lock_word_decr.h>
#include <innodb/sync_rw/rw_lock_set_writer_id_and_recursion_flag.h>
#include <innodb/sync_rw/rw_lock_x_lock_wait.h>
#include <innodb/thread/os_thread_eq.h>
#include <innodb/sync_rw/X_LOCK_DECR.h>

/** Low-level function for acquiring an exclusive lock.
 @return false if did not succeed, true if success. */
ibool rw_lock_x_lock_low(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    ulint pass,            /*!< in: pass value; != 0, if the lock will
                           be passed to another thread to unlock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line)            /*!< in: line where requested */
{
  if (rw_lock_lock_word_decr(lock, X_LOCK_DECR, X_LOCK_HALF_DECR)) {
    /* lock->recursive also tells us if the writer_thread
    field is stale or active. As we are going to write
    our own thread id in that field it must be that the
    current writer_thread value is not active. */
    ut_a(!lock->recursive);

    /* Decrement occurred: we are writer or next-writer. */
    rw_lock_set_writer_id_and_recursion_flag(lock, !pass);

    rw_lock_x_lock_wait(lock, pass, 0, file_name, line);

  } else {
    os_thread_id_t thread_id = os_thread_get_curr_id();

    if (!pass) {
      os_rmb;
    }

    /* Decrement failed: An X or SX lock is held by either
    this thread or another. Try to relock. */
    if (!pass && lock->recursive &&
        os_thread_eq(lock->writer_thread, thread_id)) {
      /* Other s-locks can be allowed. If it is request x
      recursively while holding sx lock, this x lock should
      be along with the latching-order. */

      /* The existing X or SX lock is from this thread */
      if (rw_lock_lock_word_decr(lock, X_LOCK_DECR, 0)) {
        /* There is at least one SX-lock from this
        thread, but no X-lock. */

        /* Wait for any the other S-locks to be
        released. */
        rw_lock_x_lock_wait(lock, pass, -X_LOCK_HALF_DECR, file_name, line);

      } else {
        /* At least one X lock by this thread already
        exists. Add another. */
        if (lock->lock_word == 0 || lock->lock_word == -X_LOCK_HALF_DECR) {
          lock->lock_word -= X_LOCK_DECR;
        } else {
          ut_ad(lock->lock_word <= -X_LOCK_DECR);
          --lock->lock_word;
        }
      }

    } else {
      /* Another thread locked before us */
      return (FALSE);
    }
  }

  ut_d(rw_lock_add_debug_info(lock, pass, RW_LOCK_X, file_name, line));

  lock->last_x_file_name = file_name;
  lock->last_x_line = (unsigned int)line;

  return (TRUE);
}
