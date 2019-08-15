#include <innodb/dict_mem/dict_table_move_from_non_lru_to_lru.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_non_lru_find_table.h>
#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/dict_mem/dict_sys.h>

/** Move a table to the LRU end from the non LRU list.
@param[in]	table	InnoDB table object */
void dict_table_move_from_non_lru_to_lru(dict_table_t *table) {
  ut_ad(mutex_own(&dict_sys->mutex));
  ut_ad(dict_non_lru_find_table(table));

  ut_a(!table->can_be_evicted);

  UT_LIST_REMOVE(dict_sys->table_non_LRU, table);

  UT_LIST_ADD_LAST(dict_sys->table_LRU, table);

  table->can_be_evicted = TRUE;
}
