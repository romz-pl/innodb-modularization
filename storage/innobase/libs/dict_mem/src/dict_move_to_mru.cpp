#include <innodb/dict_mem/dict_move_to_mru.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_lru_validate.h>
#include <innodb/dict_mem/dict_lru_find_table.h>
#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/dict_mem/dict_sys.h>

/** Move to the most recently used segment of the LRU list. */
void dict_move_to_mru(dict_table_t *table) /*!< in: table to move to MRU */
{
  ut_ad(mutex_own(&dict_sys->mutex));
  ut_ad(dict_lru_validate());
  ut_ad(dict_lru_find_table(table));

  ut_a(table->can_be_evicted);

  UT_LIST_REMOVE(dict_sys->table_LRU, table);

  UT_LIST_ADD_FIRST(dict_sys->table_LRU, table);

  ut_ad(dict_lru_validate());
}
