#include <innodb/lock_priv/lock_rec_get_first_on_page.h>

#include <innodb/buf_block/buf_block_t.h>
#include <innodb/buf_block/buf_block_get_lock_hash_val.h>
#include <innodb/hash/HASH_GET_FIRST.h>
#include <innodb/hash/HASH_GET_NEXT.h>
#include <innodb/lock_priv/lock_t.h>


/** Gets the first record lock on a page, where the page is identified by a
 pointer to it.
 @return	first lock, NULL if none exists */
lock_t *lock_rec_get_first_on_page(
    hash_table_t *lock_hash,  /*!< in: lock hash table */
    const buf_block_t *block) /*!< in: buffer block */
{
  ut_ad(lock_mutex_own());

  space_id_t space = block->page.id.space();
  page_no_t page_no = block->page.id.page_no();
  ulint hash = buf_block_get_lock_hash_val(block);

  for (lock_t *lock = static_cast<lock_t *>(HASH_GET_FIRST(lock_hash, hash));
       lock != NULL; lock = static_cast<lock_t *>(HASH_GET_NEXT(hash, lock))) {
    if (lock->space_id() == space && lock->page_no() == page_no) {
      return (lock);
    }
  }

  return (NULL);
}
