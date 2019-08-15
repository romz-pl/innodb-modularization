#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/sync_mutex/mutex_free.h>

/** Destroy the mutex of a given table.
This function is only called from either single threaded environment
or from a thread that has not shared the table object with other threads.
@param[in,out]	table	table whose mutex is to be created */
inline void dict_table_mutex_destroy(dict_table_t *table) {
  if (table->mutex_created == os_once::DONE) {
    if (table->mutex != nullptr) {
      mutex_free(table->mutex);
      UT_DELETE(table->mutex);
    }
  }
}
