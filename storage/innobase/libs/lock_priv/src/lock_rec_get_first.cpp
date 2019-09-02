#include <innodb/lock_priv/lock_rec_get_first.h>

#include <innodb/lock_sys/lock_mutex_own.h>
#include <innodb/lock_priv/lock_rec_get_first_on_page.h>
#include <innodb/lock_priv/lock_rec_get_next_on_page.h>
#include <innodb/lock_priv/lock_rec_get_nth_bit.h>



/** Gets the first explicit lock request on a record.
 @return	first lock, NULL if none exists */
lock_t *lock_rec_get_first(
    hash_table_t *hash,       /*!< in: hash chain the lock on */
    const buf_block_t *block, /*!< in: block containing the record */
    ulint heap_no)            /*!< in: heap number of the record */
{
  ut_ad(lock_mutex_own());

  for (lock_t *lock = lock_rec_get_first_on_page(hash, block); lock;
       lock = lock_rec_get_next_on_page(lock)) {
    if (lock_rec_get_nth_bit(lock, heap_no)) {
      return (lock);
    }
  }

  return (NULL);
}
