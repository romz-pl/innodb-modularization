#include <innodb/hash/hash_get_mutex.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_table_t.h>
#include <innodb/hash/hash_get_sync_obj_index.h>
#include <innodb/hash/hash_get_nth_mutex.h>

/** Gets the mutex for a fold value in a hash table.
 @return mutex */
ib_mutex_t *hash_get_mutex(hash_table_t *table, /*!< in: hash table */
                           ulint fold)          /*!< in: fold */
{
  ulint i;

  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);

  i = hash_get_sync_obj_index(table, fold);

  return (hash_get_nth_mutex(table, i));
}

#endif
