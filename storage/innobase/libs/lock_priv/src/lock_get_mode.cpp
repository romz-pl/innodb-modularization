#include <innodb/lock_priv/lock_get_mode.h>

#include <innodb/lock_priv/lock_t.h>

/** Gets the mode of a lock.
 @return mode */
enum lock_mode lock_get_mode(const lock_t *lock) /*!< in: lock */
{
  ut_ad(lock);

  return (static_cast<enum lock_mode>(lock->type_mode & LOCK_MODE_MASK));
}
