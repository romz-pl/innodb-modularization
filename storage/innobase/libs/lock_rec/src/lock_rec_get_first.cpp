#include <innodb/lock_rec/lock_rec_get_first.h>

#include <innodb/lock_sys/lock_mutex_own.h>
#include <innodb/lock_priv/lock_rec_get_first_on_page.h>
#include <innodb/lock_priv/lock_rec_get_next_on_page.h>
#include <innodb/lock_priv/lock_rec_get_nth_bit.h>
#include <innodb/lock_rec/RecID.h>
#include <innodb/lock_priv/lock_rec_get_first_on_page_addr.h>
#include <innodb/lock_priv/lock_rec_get_next.h>

/** Gets the first explicit lock request on a record.
@param[in]	hash		Record hash
@param[in]	rec_id		Record ID
@return	first lock, nullptr if none exists */
lock_t *lock_rec_get_first(hash_table_t *hash, const RecID &rec_id) {
  ut_ad(lock_mutex_own());

  auto lock = lock_rec_get_first_on_page_addr(hash, rec_id.m_space_id,
                                              rec_id.m_page_no);

  if (lock != nullptr && !lock_rec_get_nth_bit(lock, rec_id.m_heap_no)) {
    lock = lock_rec_get_next(rec_id.m_heap_no, lock);
  }

  return (lock);
}

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



