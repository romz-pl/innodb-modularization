#include <innodb/lock_priv/lock_rec_get_insert_intention.h>

#include <innodb/lock_priv/lock_get_type_low.h>
#include <innodb/lock_priv/flags.h>
#include <innodb/assert/assert.h>
#include <innodb/lock_priv/lock_t.h>

/** Gets the waiting insert flag of a record lock.
 @return LOCK_INSERT_INTENTION or 0 */
ulint lock_rec_get_insert_intention(const lock_t *lock) /*!< in: record lock */
{
  ut_ad(lock_get_type_low(lock) == LOCK_REC);

  return (lock->type_mode & LOCK_INSERT_INTENTION);
}
