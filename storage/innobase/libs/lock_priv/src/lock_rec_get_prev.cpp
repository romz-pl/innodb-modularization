#include <innodb/lock_priv/lock_rec_get_prev.h>

#include <innodb/lock_sys/lock_mutex_own.h>
#include <innodb/lock_priv/lock_get_type_low.h>
#include <innodb/lock_priv/lock_hash_get.h>
#include <innodb/lock_priv/lock_rec_get_first_on_page_addr.h>
#include <innodb/lock_priv/lock_rec_get_next_on_page.h>
#include <innodb/lock_priv/lock_rec_get_nth_bit.h>
#include <innodb/lock_priv/lock_t.h>

/** Gets the previous record lock set on a record.
 @return previous lock on the same record, NULL if none exists */
const lock_t *lock_rec_get_prev(
    const lock_t *in_lock, /*!< in: record lock */
    ulint heap_no)         /*!< in: heap number of the record */
{
  lock_t *lock;
  space_id_t space;
  page_no_t page_no;
  lock_t *found_lock = NULL;
  hash_table_t *hash;

  ut_ad(lock_mutex_own());
  ut_ad(lock_get_type_low(in_lock) == LOCK_REC);

  space = in_lock->rec_lock.space;
  page_no = in_lock->rec_lock.page_no;

  hash = lock_hash_get(in_lock->type_mode);

  for (lock = lock_rec_get_first_on_page_addr(hash, space, page_no);
       /* No op */; lock = lock_rec_get_next_on_page(lock)) {
    ut_ad(lock);

    if (lock == in_lock) {
      return (found_lock);
    }

    if (lock_rec_get_nth_bit(lock, heap_no)) {
      found_lock = lock;
    }
  }
}
