#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_index_get_lock.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Obtain exclusive locks on all index trees of the table. This is to prevent
 accessing index trees while InnoDB is updating internal metadata for
 operations such as FLUSH TABLES. */
UNIV_INLINE
void dict_table_x_lock_indexes(dict_table_t *table) /*!< in: table */
{
  dict_index_t *index;

  ut_a(table);
  ut_ad(mutex_own(&dict_sys->mutex));

  /* Loop through each index of the table and lock them */
  for (index = table->first_index(); index != NULL; index = index->next()) {
    rw_lock_x_lock(dict_index_get_lock(index));
  }
}
