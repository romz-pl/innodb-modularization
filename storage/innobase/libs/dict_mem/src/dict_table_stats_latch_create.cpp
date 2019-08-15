#include <innodb/dict_mem/dict_table_stats_latch_create.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Create a dict_table_t's stats latch or delay for lazy creation.
This function is only called from either single threaded environment
or from a thread that has not shared the table object with other threads.
@param[in,out]	table	table whose stats latch to create
@param[in]	enabled	if false then the latch is disabled
and dict_table_stats_lock()/unlock() become noop on this table. */
void dict_table_stats_latch_create(dict_table_t *table, bool enabled) {
  if (!enabled) {
    table->stats_latch = NULL;
    table->stats_latch_created = os_once::DONE;
    return;
  }

  /* We create this lazily the first time it is used. */
  table->stats_latch = NULL;
  table->stats_latch_created = os_once::NEVER_DONE;
}
