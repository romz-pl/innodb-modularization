#include <innodb/hash/hash_calc_hash.h>

#include <innodb/hash/hash_table_t.h>
#include <innodb/random/random.h>

/** Calculates the hash value from a folded value.
 @return hashed value */
ulint hash_calc_hash(ulint fold,          /*!< in: folded value */
                     hash_table_t *table) /*!< in: hash table */
{
  ut_ad(table);
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);
  return (ut_hash_ulint(fold, table->n_cells));
}
