#include <innodb/sync_rw/rw_lock_sx_lock_low.h>

#include <innodb/sync_rw/X_LOCK_HALF_DECR.h>
#include <innodb/sync_rw/rw_lock_set_writer_id_and_recursion_flag.h>
#include <innodb/thread/os_thread_eq.h>
#include <innodb/sync_rw/rw_lock_lock_word_decr.h>

/** Low-level function for acquiring an sx lock.
 @return false if did not succeed, true if success. */
ibool rw_lock_sx_lock_low(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    ulint pass,            /*!< in: pass value; != 0, if the lock will
                           be passed to another thread to unlock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line)            /*!< in: line where requested */
{
  if (rw_lock_lock_word_decr(lock, X_LOCK_HALF_DECR, X_LOCK_HALF_DECR)) {
    /* lock->recursive also tells us if the writer_thread
    field is stale or active. As we are going to write
    our own thread id in that field it must be that the
    current writer_thread value is not active. */
    ut_a(!lock->recursive);

    /* Decrement occurred: we are the SX lock owner. */
    rw_lock_set_writer_id_and_recursion_flag(lock, !pass);

    lock->sx_recursive = 1;

  } else {
    os_thread_id_t thread_id = os_thread_get_curr_id();

    if (!pass) {
      os_rmb;
    }

    /* Decrement failed: It already has an X or SX lock by this
    thread or another thread. If it is this thread, relock,
    else fail. */
    if (!pass && lock->recursive &&
        os_thread_eq(lock->writer_thread, thread_id)) {
      /* This thread owns an X or SX lock */
      if (lock->sx_recursive++ == 0) {
        /* This thread is making first SX-lock request
        and it must be holding at least one X-lock here
        because:

        * There can't be a WAIT_EX thread because we are
          the thread which has it's thread_id written in
          the writer_thread field and we are not waiting.

        * Any other X-lock thread cannot exist because
          it must update recursive flag only after
          updating the thread_id. Had there been
          a concurrent X-locking thread which succeeded
          in decrementing the lock_word it must have
          written it's thread_id before setting the
          recursive flag. As we cleared the if()
          condition above therefore we must be the only
          thread working on this lock and it is safe to
          read and write to the lock_word. */

        ut_ad((lock->lock_word == 0) ||
              ((lock->lock_word <= -X_LOCK_DECR) &&
               (lock->lock_word > -(X_LOCK_DECR + X_LOCK_HALF_DECR))));
        lock->lock_word -= X_LOCK_HALF_DECR;
      }
    } else {
      /* Another thread locked before us */
      return (FALSE);
    }
  }

  ut_d(rw_lock_add_debug_info(lock, pass, RW_LOCK_SX, file_name, line));

  lock->last_x_file_name = file_name;
  lock->last_x_line = (unsigned int)line;

  return (TRUE);
}

