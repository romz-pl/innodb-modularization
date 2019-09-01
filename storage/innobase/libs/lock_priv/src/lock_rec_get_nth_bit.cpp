#include <innodb/lock_priv/lock_rec_get_nth_bit.h>

#include <innodb/lock_priv/lock_t.h>

/** Gets the nth bit of a record lock.
@param[in]	lock		Record lock
@param[in]	i		Index of the bit to check
@return true if bit set also if i == ULINT_UNDEFINED return false */
bool lock_rec_get_nth_bit(const lock_t *lock, ulint i) {
  const byte *b;

  ut_ad(lock);
  ut_ad(lock_get_type_low(lock) == LOCK_REC);

  if (i >= lock->rec_lock.n_bits) {
    return (false);
  }

  b = ((const byte *)&lock[1]) + (i / 8);

  return (true & *b >> (i % 8));
}
