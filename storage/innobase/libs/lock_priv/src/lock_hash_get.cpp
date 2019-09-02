#include <innodb/lock_priv/lock_hash_get.h>

#include <innodb/lock_sys/lock_sys.h>
#include <innodb/lock_priv/flags.h>

/** Get the lock hash table */
hash_table_t *lock_hash_get(ulint mode) /*!< in: lock mode */
{
  if (mode & LOCK_PREDICATE) {
    return (lock_sys->prdt_hash);
  } else if (mode & LOCK_PRDT_PAGE) {
    return (lock_sys->prdt_page_hash);
  } else {
    return (lock_sys->rec_hash);
  }
}
