#include <innodb/lock_priv/lock_rec_bitmap_reset.h>

#include <innodb/lock_priv/lock_get_type_low.h>
#include <innodb/lock_priv/lock_rec_get_n_bits.h>
#include <innodb/assert/assert.h>
#include <innodb/lock_priv/lock_t.h>


/** Resets the record lock bitmap to zero. NOTE: does not touch the wait_lock
 pointer in the transaction! This function is used in lock object creation
 and resetting. */
void lock_rec_bitmap_reset(lock_t *lock) /*!< in: record lock */
{
  ulint n_bytes;

  ut_ad(lock_get_type_low(lock) == LOCK_REC);

  /* Reset to zero the bitmap which resides immediately after the lock
  struct */

  n_bytes = lock_rec_get_n_bits(lock) / 8;

  ut_ad((lock_rec_get_n_bits(lock) % 8) == 0);

  memset(&lock[1], 0, n_bytes);
}
