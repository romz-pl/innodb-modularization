#include <innodb/lock_priv/lock_rec_get_next_const.h>

#include <innodb/lock_priv/lock_rec_get_next.h>

/** Gets the next explicit lock request on a record.
 @return	next lock, NULL if none exists or if heap_no == ULINT_UNDEFINED
 */
const lock_t *lock_rec_get_next_const(
    ulint heap_no,      /*!< in: heap number of the record */
    const lock_t *lock) /*!< in: lock */
{
  return (lock_rec_get_next(heap_no, (lock_t *)lock));
}
