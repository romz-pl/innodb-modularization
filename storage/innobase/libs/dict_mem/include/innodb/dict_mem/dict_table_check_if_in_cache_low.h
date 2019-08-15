#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/hash/HASH_SEARCH.h>
#include <innodb/dict_mem/dict_sys.h>

/** Checks if a table is in the dictionary cache.
 @return table, NULL if not found */
UNIV_INLINE
dict_table_t *dict_table_check_if_in_cache_low(
    const char *table_name) /*!< in: table name */
{
  dict_table_t *table;
  ulint table_fold;

  DBUG_ENTER("dict_table_check_if_in_cache_low");
  DBUG_PRINT("dict_table_check_if_in_cache_low", ("table: '%s'", table_name));

  ut_ad(table_name);
  ut_ad(mutex_own(&dict_sys->mutex));

  /* Look for the table name in the hash table */
  table_fold = ut_fold_string(table_name);

  HASH_SEARCH(name_hash, dict_sys->table_hash, table_fold, dict_table_t *,
              table, ut_ad(table->cached),
              !strcmp(table->name.m_name, table_name));
  DBUG_RETURN(table);
}
