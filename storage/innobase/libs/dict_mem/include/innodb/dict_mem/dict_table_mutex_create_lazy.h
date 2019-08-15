#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Request for lazy creation of the mutex of a given table.
This function is only called from either single threaded environment
or from a thread that has not shared the table object with other threads.
@param[in,out]	table	table whose mutex is to be created */
inline void dict_table_mutex_create_lazy(dict_table_t *table) {
  table->mutex = nullptr;
  table->mutex_created = os_once::NEVER_DONE;
}
