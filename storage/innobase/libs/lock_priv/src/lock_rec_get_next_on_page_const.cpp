#include <innodb/lock_priv/lock_rec_get_next_on_page_const.h>

#include <innodb/lock_sys/lock_mutex_own.h>
#include <innodb/lock_priv/lock_get_type_low.h>
#include <innodb/univ/page_no_t.h>
#include <innodb/hash/HASH_GET_NEXT.h>
#include <innodb/lock_priv/lock_t.h>

/** Gets the first or next record lock on a page.
 @return next lock, NULL if none exists */
const lock_t *lock_rec_get_next_on_page_const(
    const lock_t *lock) /*!< in: a record lock */
{
  ut_ad(lock_mutex_own());
  ut_ad(lock_get_type_low(lock) == LOCK_REC);

  space_id_t space = lock->space_id();
  page_no_t page_no = lock->page_no();

  while ((lock = static_cast<const lock_t *>(HASH_GET_NEXT(hash, lock))) !=
         NULL) {
    if (lock->space_id() == space && lock->page_no() == page_no) {
      return (lock);
    }
  }

  return (NULL);
}
