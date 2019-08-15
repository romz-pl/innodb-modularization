#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/sync_mutex/mutex_free.h>

/** Destroy the autoinc latch of the given table.
This function is only called from either single threaded environment
or from a thread that has not shared the table object with other threads.
@param[in,out]	table	table whose stats latch to destroy */
inline void dict_table_autoinc_destroy(dict_table_t *table) {
  if (table->autoinc_mutex_created == os_once::DONE) {
    if (table->autoinc_mutex != NULL) {
      mutex_free(table->autoinc_mutex);
      UT_DELETE(table->autoinc_mutex);
    }

    if (table->autoinc_persisted_mutex != NULL) {
      mutex_free(table->autoinc_persisted_mutex);
      UT_DELETE(table->autoinc_persisted_mutex);
    }
  }
}
