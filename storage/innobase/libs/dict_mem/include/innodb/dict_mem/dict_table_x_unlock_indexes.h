#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_index_get_lock.h>
#include <innodb/sync_mutex/mutex_own.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>

/** Release the exclusive locks on all index tree. */
UNIV_INLINE
void dict_table_x_unlock_indexes(dict_table_t *table) /*!< in: table */
{
  dict_index_t *index;

  ut_a(table);
  ut_ad(mutex_own(&dict_sys->mutex));

  for (index = table->first_index(); index != NULL; index = index->next()) {
    rw_lock_x_unlock(dict_index_get_lock(index));
  }
}
