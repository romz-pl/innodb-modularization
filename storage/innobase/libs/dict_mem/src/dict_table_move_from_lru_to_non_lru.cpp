#include <innodb/dict_mem/dict_table_move_from_lru_to_non_lru.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_lru_find_table.h>
#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/dict_mem/dict_sys.h>

/** Move a table to the non-LRU list from the LRU list. */
void dict_table_move_from_lru_to_non_lru(
    dict_table_t *table) /*!< in: table to move from LRU to non-LRU */
{
  ut_ad(mutex_own(&dict_sys->mutex));
  ut_ad(dict_lru_find_table(table));

  ut_a(table->can_be_evicted);

  UT_LIST_REMOVE(dict_sys->table_LRU, table);

  UT_LIST_ADD_LAST(dict_sys->table_non_LRU, table);

  table->can_be_evicted = FALSE;
}
