#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_allow_eviction.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Move this table to LRU list after DDL operations if it was moved
to non-LRU list
@param[in,out]	table	Table to put in LRU list */
UNIV_INLINE
void dict_table_ddl_release(dict_table_t *table) {
  ut_ad(mutex_own(&dict_sys->mutex));
  if (table->ddl_not_evictable) {
    ut_ad(!table->can_be_evicted);
    dict_table_allow_eviction(table);
    table->ddl_not_evictable = false;
  }
}
