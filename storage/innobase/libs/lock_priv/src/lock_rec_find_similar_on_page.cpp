#include <innodb/lock_priv/lock_rec_find_similar_on_page.h>

#include <innodb/lock_priv/lock_rec_get_next_on_page.h>
#include <innodb/lock_priv/lock_rec_get_n_bits.h>
#include <innodb/lock_sys/lock_mutex_own.h>
#include <innodb/lock_priv/lock_t.h>

/** Looks for a suitable type record lock struct by the same trx on the same
 page. This can be used to save space when a new record lock should be set on a
 page: no new struct is needed, if a suitable old is found.
 @return lock or NULL */
lock_t *lock_rec_find_similar_on_page(
    ulint type_mode,  /*!< in: lock type_mode field */
    ulint heap_no,    /*!< in: heap number of the record */
    lock_t *lock,     /*!< in: lock_rec_get_first_on_page() */
    const trx_t *trx) /*!< in: transaction */
{
  ut_ad(lock_mutex_own());

  for (/* No op */; lock != NULL; lock = lock_rec_get_next_on_page(lock)) {
    if (lock->trx == trx && lock->type_mode == type_mode &&
        lock_rec_get_n_bits(lock) > heap_no) {
      return (lock);
    }
  }

  return (NULL);
}
