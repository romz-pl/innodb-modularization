#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_move_from_non_lru_to_lru.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Allow the table to be evicted by moving a table to the LRU list from
the non-LRU list if it is not already there.
@param[in]      table   InnoDB table object can be evicted */
UNIV_INLINE
void dict_table_allow_eviction(dict_table_t *table) {
  ut_ad(mutex_own(&dict_sys->mutex));
  if (!table->can_be_evicted) {
    dict_table_move_from_non_lru_to_lru(table);
  }
  table->explicitly_non_lru = false;
}
