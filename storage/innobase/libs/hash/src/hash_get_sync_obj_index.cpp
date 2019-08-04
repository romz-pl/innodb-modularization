#include <innodb/hash/hash_get_sync_obj_index.h>

#include <innodb/hash/hash_table_t.h>
#include <innodb/math/ut_is_2pow.h>
#include <innodb/math/ut_2pow_remainder.h>
#include <innodb/hash/hash_calc_hash.h>

#ifndef UNIV_HOTBACKUP

/** Gets the sync object index for a fold value in a hash table.
 @return index */
ulint hash_get_sync_obj_index(hash_table_t *table, /*!< in: hash table */
                              ulint fold)          /*!< in: fold */
{
  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
  ut_ad(table->type != HASH_TABLE_SYNC_NONE);
  ut_ad(ut_is_2pow(table->n_sync_obj));
  return (ut_2pow_remainder(hash_calc_hash(fold, table), table->n_sync_obj));
}

#endif
