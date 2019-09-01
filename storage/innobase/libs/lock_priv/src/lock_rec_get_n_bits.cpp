#include <innodb/lock_priv/lock_rec_get_n_bits.h>

#include <innodb/lock_priv/lock_t.h>

/** Gets the number of bits in a record lock bitmap.
 @return	number of bits */
ulint lock_rec_get_n_bits(const lock_t *lock) /*!< in: record lock */
{
  return (lock->rec_lock.n_bits);
}
