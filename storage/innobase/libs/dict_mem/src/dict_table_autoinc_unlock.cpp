#include <innodb/dict_mem/dict_table_autoinc_unlock.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/sync_mutex/mutex_exit.h>

/** Release the autoinc lock. */
void dict_table_autoinc_unlock(dict_table_t *table) /*!< in/out: table */
{
  mutex_exit(table->autoinc_mutex);
}
