#include <innodb/lock_priv/lock_rec_get_first_on_page_addr.h>

#include <innodb/lock_priv/lock_rec_hash.h>
#include <innodb/lock_sys/lock_mutex_own.h>
#include <innodb/hash/HASH_GET_NEXT.h>
#include <innodb/hash/HASH_GET_FIRST.h>
#include <innodb/lock_priv/lock_t.h>


/** Gets the first record lock on a page, where the page is identified by its
 file address.
 @return	first lock, NULL if none exists */
lock_t *lock_rec_get_first_on_page_addr(
    hash_table_t *lock_hash, /*!< in: Lock hash table */
    space_id_t space,        /*!< in: space */
    page_no_t page_no)       /*!< in: page number */
{
  ut_ad(lock_mutex_own());

  for (lock_t *lock = static_cast<lock_t *>(
           HASH_GET_FIRST(lock_hash, lock_rec_hash(space, page_no)));
       lock != NULL; lock = static_cast<lock_t *>(HASH_GET_NEXT(hash, lock))) {
    if (lock->space_id() == space && lock->page_no() == page_no) {
      return (lock);
    }
  }

  return (NULL);
}
