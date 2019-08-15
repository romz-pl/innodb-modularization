#include <innodb/dict_mem/dict_table_stats_latch_destroy.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_stats_latch_free.h>

/** Destroy a dict_table_t's stats latch.
This function is only called from either single threaded environment
or from a thread that has not shared the table object with other threads.
@param[in,out]	table	table whose stats latch to destroy */
void dict_table_stats_latch_destroy(dict_table_t *table) {
  if (table->stats_latch_created == os_once::DONE &&
      table->stats_latch != NULL) {
    dict_table_stats_latch_free(table);
  }
}
