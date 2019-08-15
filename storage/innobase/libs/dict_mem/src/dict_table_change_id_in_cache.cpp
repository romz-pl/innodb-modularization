#include <innodb/dict_mem/dict_table_change_id_in_cache.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/hash/HASH_DELETE.h>
#include <innodb/hash/HASH_INSERT.h>
#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/random/random.h>
#include <innodb/dict_mem/dict_sys.h>

/** Change the id of a table object in the dictionary cache. This is used in
 DISCARD TABLESPACE. */
void dict_table_change_id_in_cache(
    dict_table_t *table, /*!< in/out: table object already in cache */
    table_id_t new_id)   /*!< in: new id to set */
{
  ut_ad(table);
  ut_ad(mutex_own(&dict_sys->mutex));
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  /* Remove the table from the hash table of id's */

  HASH_DELETE(dict_table_t, id_hash, dict_sys->table_id_hash,
              ut_fold_ull(table->id), table);
  table->id = new_id;

  /* Add the table back to the hash table */
  HASH_INSERT(dict_table_t, id_hash, dict_sys->table_id_hash,
              ut_fold_ull(table->id), table);
}
