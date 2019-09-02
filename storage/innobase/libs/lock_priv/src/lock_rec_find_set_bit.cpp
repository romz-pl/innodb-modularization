#include <innodb/lock_priv/lock_rec_find_set_bit.h>

#include <innodb/lock_priv/lock_rec_get_n_bits.h>
#include <innodb/lock_priv/lock_rec_get_nth_bit.h>

/** Looks for a set bit in a record lock bitmap. Returns ULINT_UNDEFINED,
 if none found.
 @return bit index == heap number of the record, or ULINT_UNDEFINED if
 none found */
ulint lock_rec_find_set_bit(
    const lock_t *lock) /*!< in: record lock with at least one bit set */
{
  for (ulint i = 0; i < lock_rec_get_n_bits(lock); ++i) {
    if (lock_rec_get_nth_bit(lock, i)) {
      return (i);
    }
  }

  return (ULINT_UNDEFINED);
}
