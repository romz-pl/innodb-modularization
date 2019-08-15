#include <innodb/dict_mem/dict_non_lru_find_table.h>


#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

#include <innodb/dict_mem/dict_table_t.h>

/** Check if a table exists in the dict table non-LRU list.
 @return true if table found in non-LRU list */
ibool dict_non_lru_find_table(
    const dict_table_t *find_table) /*!< in: table to find */
{
  dict_table_t *table;

  ut_ad(find_table != NULL);
  ut_ad(mutex_own(&dict_sys->mutex));

  for (table = UT_LIST_GET_FIRST(dict_sys->table_non_LRU); table != NULL;
       table = UT_LIST_GET_NEXT(table_LRU, table)) {
    ut_a(!table->can_be_evicted);

    if (table == find_table) {
      return (TRUE);
    }
  }

  return (FALSE);
}

#endif
#endif
