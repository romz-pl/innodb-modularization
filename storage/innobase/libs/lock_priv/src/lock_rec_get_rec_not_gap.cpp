#include <innodb/lock_priv/lock_rec_get_rec_not_gap.h>

#include <innodb/lock_priv/lock_get_type_low.h>
#include <innodb/lock_priv/flags.h>
#include <innodb/assert/assert.h>
#include <innodb/lock_priv/lock_t.h>

/** Gets the LOCK_REC_NOT_GAP flag of a record lock.
 @return LOCK_REC_NOT_GAP or 0 */
ulint lock_rec_get_rec_not_gap(const lock_t *lock) /*!< in: record lock */
{
  ut_ad(lock_get_type_low(lock) == LOCK_REC);

  return (lock->type_mode & LOCK_REC_NOT_GAP);
}
