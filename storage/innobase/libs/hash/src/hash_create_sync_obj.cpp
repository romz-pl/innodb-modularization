#include <innodb/hash/hash_create_sync_obj.h>


#ifndef UNIV_HOTBACKUP

#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/error/ut_error.h>
#include <innodb/hash/hash_table_t.h>
#include <innodb/math/ut_is_2pow.h>
#include <innodb/sync_latch/sync_latch_get_level.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_os/pfs.h>
#include <innodb/sync_rw/rw_lock_create.h>


/** Creates a sync object array to protect a hash table. "::sync_obj" can be
mutexes or rw_locks depening on the type of hash table.
@param[in]	table		hash table
@param[in]	type		HASH_TABLE_SYNC_MUTEX or
                                HASH_TABLE_SYNC_RW_LOCK
@param[in]	id		latch ID
@param[in]	n_sync_obj	number of sync objects, must be a power of 2 */
void hash_create_sync_obj(hash_table_t *table, enum hash_table_sync_t type,
                          latch_id_t id, ulint n_sync_obj) {
  ut_a(n_sync_obj > 0);
  ut_a(ut_is_2pow(n_sync_obj));
  ut_ad(table->magic_n == HASH_TABLE_MAGIC_N);

  table->type = type;

  switch (table->type) {
    case HASH_TABLE_SYNC_MUTEX:
      table->sync_obj.mutexes = static_cast<ib_mutex_t *>(
          ut_malloc_nokey(n_sync_obj * sizeof(ib_mutex_t)));

      for (ulint i = 0; i < n_sync_obj; i++) {
        mutex_create(id, table->sync_obj.mutexes + i);
      }

      break;

    case HASH_TABLE_SYNC_RW_LOCK: {
      latch_level_t level = sync_latch_get_level(id);

      ut_a(level != SYNC_UNKNOWN);

      table->sync_obj.rw_locks = static_cast<rw_lock_t *>(
          ut_malloc_nokey(n_sync_obj * sizeof(rw_lock_t)));

      for (ulint i = 0; i < n_sync_obj; i++) {
        rw_lock_create(hash_table_locks_key, table->sync_obj.rw_locks + i,
                       level);
      }

      break;
    }

    case HASH_TABLE_SYNC_NONE:
      ut_error;
  }

  table->n_sync_obj = n_sync_obj;
}


#endif /* !UNIV_HOTBACKUP */
