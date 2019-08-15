#include <innodb/dict_mem/dict_table_stats_latch_alloc.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/sync_rw/rw_lock_create.h>

extern mysql_pfs_key_t dict_table_stats_key;

/** Allocate and init a dict_table_t's stats latch.
This function must not be called concurrently on the same table object.
@param[in,out]	table_void	table whose stats latch to create */
void dict_table_stats_latch_alloc(void *table_void) {
  dict_table_t *table = static_cast<dict_table_t *>(table_void);

  /* Note: rw_lock_create() will call the constructor */

  table->stats_latch =
      static_cast<rw_lock_t *>(ut_malloc_nokey(sizeof(rw_lock_t)));

  ut_a(table->stats_latch != NULL);

  rw_lock_create(dict_table_stats_key, table->stats_latch, SYNC_INDEX_TREE);
}
