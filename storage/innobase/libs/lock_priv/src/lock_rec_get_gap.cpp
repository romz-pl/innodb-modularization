#include <innodb/lock_priv/lock_rec_get_gap.h>

#include <innodb/lock_priv/lock_get_type_low.h>
#include <innodb/lock_priv/flags.h>
#include <innodb/assert/assert.h>
#include <innodb/lock_priv/lock_t.h>

/** Gets the gap flag of a record lock.
 @return LOCK_GAP or 0 */
ulint lock_rec_get_gap(const lock_t *lock) /*!< in: record lock */
{
  ut_ad(lock_get_type_low(lock) == LOCK_REC);

  return (lock->type_mode & LOCK_GAP);
}
