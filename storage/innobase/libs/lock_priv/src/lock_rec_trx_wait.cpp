#include <innodb/lock_priv/lock_rec_trx_wait.h>

#include <innodb/lock_priv/lock_rec_reset_nth_bit.h>
#include <innodb/lock_priv/lock_reset_lock_and_trx_wait.h>
#include <innodb/lock_priv/flags.h>

/** Reset the nth bit of a record lock.
@param[in,out]	lock record lock
@param[in] i	index of the bit that will be reset
@param[in] type	whether the lock is in wait mode */
void lock_rec_trx_wait(lock_t *lock, ulint i, ulint type) {
  lock_rec_reset_nth_bit(lock, i);

  if (type & LOCK_WAIT) {
    lock_reset_lock_and_trx_wait(lock);
  }
}
