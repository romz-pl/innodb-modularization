#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_move_from_lru_to_non_lru.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Prevent table eviction by moving a table to the non-LRU list from the
 LRU list if it is not already there. */
UNIV_INLINE
void dict_table_prevent_eviction(
    dict_table_t *table) /*!< in: table to prevent eviction */
{
  ut_ad(mutex_own(&dict_sys->mutex));
  if (table->can_be_evicted) {
    dict_table_move_from_lru_to_non_lru(table);
  }
  table->explicitly_non_lru = true;
}
