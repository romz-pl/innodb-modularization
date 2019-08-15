#include <innodb/dict_mem/dict_table_autoinc_lock.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/dict_mem/dict_table_autoinc_alloc.h>

/** Acquire the autoinc lock. */
void dict_table_autoinc_lock(dict_table_t *table) /*!< in/out: table */
{
  os_once::do_or_wait_for_done(&table->autoinc_mutex_created,
                               dict_table_autoinc_alloc, table);

  mutex_enter(table->autoinc_mutex);
}
