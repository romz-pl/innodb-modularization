#include <innodb/lock_priv/lock_rec_get_next.h>

#include <innodb/lock_sys/lock_mutex_own.h>
#include <innodb/lock_priv/lock_rec_get_next_on_page.h>
#include <innodb/lock_priv/lock_rec_get_nth_bit.h>

/** Gets the next explicit lock request on a record.
 @return	next lock, NULL if none exists or if heap_no == ULINT_UNDEFINED
 */
lock_t *lock_rec_get_next(ulint heap_no, /*!< in: heap number of the record */
                          lock_t *lock)  /*!< in: lock */
{
  ut_ad(lock_mutex_own());

  do {
    ut_ad(lock_get_type_low(lock) == LOCK_REC);
    lock = lock_rec_get_next_on_page(lock);
  } while (lock != nullptr && !lock_rec_get_nth_bit(lock, heap_no));

  return (lock);
}
