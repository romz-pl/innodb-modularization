#include <innodb/lock_sys/lock_sys_resize.h>

#include <innodb/hash/hash_table_t.h>
#include <innodb/lock_sys/lock_mutex_enter.h>
#include <innodb/lock_sys/lock_sys.h>
#include <innodb/hash/hash_create.h>
#include <innodb/hash/HASH_MIGRATE.h>
#include <innodb/hash/hash_table_free.h>
#include <innodb/lock_priv/lock_t.h>
#include <innodb/lock_priv/lock_rec_lock_fold.h>
#include <innodb/buf_pool/srv_buf_pool_instances.h>
#include <innodb/buf_pool/buf_pool_t.h>
#include <innodb/buf_pool/buf_pool_from_array.h>
#include <innodb/buf_page/buf_page_get_state.h>
#include <innodb/lock_priv/lock_rec_hash.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/lock_sys/lock_mutex_exit.h>


/** Resize the lock hash tables.
@param[in]	n_cells	number of slots in lock hash table */
void lock_sys_resize(ulint n_cells) {
  hash_table_t *old_hash;

  lock_mutex_enter();

  old_hash = lock_sys->rec_hash;
  lock_sys->rec_hash = hash_create(n_cells);
  HASH_MIGRATE(old_hash, lock_sys->rec_hash, lock_t, hash, lock_rec_lock_fold);
  hash_table_free(old_hash);

  old_hash = lock_sys->prdt_hash;
  lock_sys->prdt_hash = hash_create(n_cells);
  HASH_MIGRATE(old_hash, lock_sys->prdt_hash, lock_t, hash, lock_rec_lock_fold);
  hash_table_free(old_hash);

  old_hash = lock_sys->prdt_page_hash;
  lock_sys->prdt_page_hash = hash_create(n_cells);
  HASH_MIGRATE(old_hash, lock_sys->prdt_page_hash, lock_t, hash,
               lock_rec_lock_fold);
  hash_table_free(old_hash);

  /* need to update block->lock_hash_val */
  for (ulint i = 0; i < srv_buf_pool_instances; ++i) {
    buf_pool_t *buf_pool = buf_pool_from_array(i);

    mutex_enter(&buf_pool->LRU_list_mutex);
    buf_page_t *bpage;
    bpage = UT_LIST_GET_FIRST(buf_pool->LRU);

    while (bpage != NULL) {
      if (buf_page_get_state(bpage) == BUF_BLOCK_FILE_PAGE) {
        buf_block_t *block;
        block = reinterpret_cast<buf_block_t *>(bpage);

        block->lock_hash_val =
            lock_rec_hash(bpage->id.space(), bpage->id.page_no());
      }
      bpage = UT_LIST_GET_NEXT(LRU, bpage);
    }
    mutex_exit(&buf_pool->LRU_list_mutex);
  }

  lock_mutex_exit();
}

