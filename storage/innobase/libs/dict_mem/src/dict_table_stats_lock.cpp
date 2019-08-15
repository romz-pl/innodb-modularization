#include <innodb/dict_mem/dict_table_stats_lock.h>

#include <innodb/dict_mem/dict_table_stats_latch_alloc.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/sync_rw/rw_lock_s_lock.h>
#include <innodb/sync_rw/rw_lock_type_t.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>

/** Lock the appropriate latch to protect a given table's statistics.
@param[in]	table		table whose stats to lock
@param[in]	latch_mode	RW_S_LATCH or RW_X_LATCH */
void dict_table_stats_lock(dict_table_t *table, ulint latch_mode) {
  ut_ad(table != NULL);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  os_once::do_or_wait_for_done(&table->stats_latch_created,
                               dict_table_stats_latch_alloc, table);

  if (table->stats_latch == NULL) {
    /* This is a dummy table object that is private in the current
    thread and is not shared between multiple threads, thus we
    skip any locking. */
    return;
  }

  switch (latch_mode) {
    case RW_S_LATCH:
      rw_lock_s_lock(table->stats_latch);
      break;
    case RW_X_LATCH:
      rw_lock_x_lock(table->stats_latch);
      break;
    case RW_NO_LATCH:
      /* fall through */
    default:
      ut_error;
  }
}
