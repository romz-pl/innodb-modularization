#include <innodb/dict_mem/dict_table_stats_unlock.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/sync_rw/rw_lock_s_unlock.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>
#include <innodb/sync_rw/rw_lock_type_t.h>

/** Unlock the latch that has been locked by dict_table_stats_lock().
@param[in]	table		table whose stats to unlock
@param[in]	latch_mode	RW_S_LATCH or RW_X_LATCH */
void dict_table_stats_unlock(dict_table_t *table, ulint latch_mode) {
  ut_ad(table != NULL);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);

  if (table->stats_latch == NULL) {
    /* This is a dummy table object that is private in the current
    thread and is not shared between multiple threads, thus we
    skip any locking. */
    return;
  }

  switch (latch_mode) {
    case RW_S_LATCH:
      rw_lock_s_unlock(table->stats_latch);
      break;
    case RW_X_LATCH:
      rw_lock_x_unlock(table->stats_latch);
      break;
    case RW_NO_LATCH:
      /* fall through */
    default:
      ut_error;
  }
}
