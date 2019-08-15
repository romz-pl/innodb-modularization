#include <innodb/dict_mem/dict_table_autoinc_alloc.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/sync_mutex/mutex_create.h>

/** Allocate and init the autoinc latch of a given table.
This function must not be called concurrently on the same table object.
@param[in,out]	table_void	table whose autoinc latch to create */
void dict_table_autoinc_alloc(void *table_void) {
  dict_table_t *table = static_cast<dict_table_t *>(table_void);

  table->autoinc_mutex = UT_NEW_NOKEY(ib_mutex_t());
  ut_a(table->autoinc_mutex != nullptr);
  mutex_create(LATCH_ID_AUTOINC, table->autoinc_mutex);

  table->autoinc_persisted_mutex = UT_NEW_NOKEY(ib_mutex_t());
  ut_a(table->autoinc_persisted_mutex != nullptr);
  mutex_create(LATCH_ID_PERSIST_AUTOINC, table->autoinc_persisted_mutex);
}
