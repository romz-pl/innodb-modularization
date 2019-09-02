#include <innodb/lock_priv/lock_rec_find_next_set_bit.h>

#include <innodb/lock_priv/lock_rec_get_n_bits.h>
#include <innodb/lock_priv/lock_rec_get_nth_bit.h>
#include <innodb/assert/assert.h>

/** Looks for the next set bit in the record lock bitmap.
@param[in] lock		record lock with at least one bit set
@param[in] heap_no	current set bit
@return The next bit index  == heap number following heap_no, or ULINT_UNDEFINED
if none found */
ulint lock_rec_find_next_set_bit(const lock_t *lock, ulint heap_no) {
  ut_ad(heap_no != ULINT_UNDEFINED);

  for (ulint i = heap_no + 1; i < lock_rec_get_n_bits(lock); ++i) {
    if (lock_rec_get_nth_bit(lock, i)) {
      return (i);
    }
  }

  return (ULINT_UNDEFINED);
}

