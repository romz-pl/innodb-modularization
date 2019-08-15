#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_prevent_eviction.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Move this table to non-LRU list for DDL operations if it's
currently not there. This also prevents later opening table via DD objects,
when the table name in InnoDB doesn't match with DD object.
@param[in,out]	table	Table to put in non-LRU list */
UNIV_INLINE
void dict_table_ddl_acquire(dict_table_t *table) {
  ut_ad(mutex_own(&dict_sys->mutex));
  if (table->can_be_evicted) {
    ut_ad(!table->ddl_not_evictable);
    dict_table_prevent_eviction(table);
    table->ddl_not_evictable = true;
  }
}
