#pragma once

#include <innodb/univ/univ.h>


#include <innodb/dict_mem/dict_table_t.h>

/** Request for lazy creation of the autoinc latch of a given table.
This function is only called from either single threaded environment
or from a thread that has not shared the table object with other threads.
@param[in,out]	table	table whose autoinc latch is to be created. */
inline void dict_table_autoinc_create_lazy(dict_table_t *table) {
  table->autoinc_mutex = NULL;
  table->autoinc_persisted_mutex = NULL;
  table->autoinc_mutex_created = os_once::NEVER_DONE;
}
