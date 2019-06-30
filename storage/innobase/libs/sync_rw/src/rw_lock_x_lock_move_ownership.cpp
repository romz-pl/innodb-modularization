#include <innodb/sync_rw/rw_lock_x_lock_move_ownership.h>

#include <innodb/sync_rw/rw_lock_set_writer_id_and_recursion_flag.h>

/** This function is used in the insert buffer to move the ownership of an
 x-latch on a buffer frame to the current thread. The x-latch was set by
 the buffer read operation and it protected the buffer frame while the
 read was done. The ownership is moved because we want that the current
 thread is able to acquire a second x-latch which is stored in an mtr.
 This, in turn, is needed to pass the debug checks of index page
 operations. */
void rw_lock_x_lock_move_ownership(
    rw_lock_t *lock) /*!< in: lock which was x-locked in the
                     buffer read */
{
  ut_ad(rw_lock_is_locked(lock, RW_LOCK_X));

  rw_lock_set_writer_id_and_recursion_flag(lock, true);
}
