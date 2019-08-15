#include <innodb/dict_mem/dict_table_stats_latch_free.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/sync_rw/rw_lock_free.h>
#include <innodb/allocator/ut_free.h>

/** Deinit and free a dict_table_t's stats latch.
This function must not be called concurrently on the same table object.
@param[in,out]	table	table whose stats latch to free */
void dict_table_stats_latch_free(dict_table_t *table) {
  rw_lock_free(table->stats_latch);
  ut_free(table->stats_latch);
}
