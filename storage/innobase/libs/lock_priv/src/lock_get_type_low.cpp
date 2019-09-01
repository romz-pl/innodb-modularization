#include <innodb/lock_priv/lock_get_type_low.h>

#include <innodb/lock_priv/lock_t.h>

/** Gets the type of a lock.
 @return LOCK_TABLE or LOCK_REC */
uint32_t lock_get_type_low(const lock_t *lock) /*!< in: lock */
{
  ut_ad(lock);

  return (lock->type_mode & LOCK_TYPE_MASK);
}
