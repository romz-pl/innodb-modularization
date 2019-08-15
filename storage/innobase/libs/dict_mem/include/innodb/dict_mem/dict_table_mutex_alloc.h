#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/sync_mutex/mutex_create.h>

/** Allocate the mutex of the given table.
This function must not be called concurrently on the same table object.
@param[in]	table_void	table whose mutex to create */
inline void dict_table_mutex_alloc(void *table_void) {
  dict_table_t *table = static_cast<dict_table_t *>(table_void);

  table->mutex = UT_NEW_NOKEY(ib_mutex_t());
  ut_ad(table->mutex != nullptr);
  mutex_create(LATCH_ID_DICT_TABLE, table->mutex);
}
#endif /* !UNIV_HOTBACKUP */
