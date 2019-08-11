#include <innodb/buf_block/buf_block_init.h>

#include <innodb/buf_pool/buf_pool_t.h>
#include <innodb/buf_block/buf_block_t.h>
#include <innodb/buf_pool/buf_pool_index.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_rw/rw_lock_create.h>
#include <innodb/sync_os/pfs.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/page_zip_des_init.h>

/** Initializes a buffer control block when the buf_pool is created. */
void buf_block_init(
    buf_pool_t *buf_pool, /*!< in: buffer pool instance */
    buf_block_t *block,   /*!< in: pointer to control block */
    byte *frame)          /*!< in: pointer to buffer frame */
{
  UNIV_MEM_DESC(frame, UNIV_PAGE_SIZE);

  /* This function should only be executed at database startup or by
  buf_pool_resize(). Either way, adaptive hash index must not exist. */
  assert_block_ahi_empty_on_init(block);

  block->frame = frame;

  block->page.buf_pool_index = buf_pool_index(buf_pool);
  block->page.state = BUF_BLOCK_NOT_USED;
  block->page.buf_fix_count = 0;
  block->page.io_fix = BUF_IO_NONE;
  block->page.flush_observer = NULL;

  block->modify_clock = 0;

  ut_d(block->page.file_page_was_freed = FALSE);

  block->index = NULL;
  block->made_dirty_with_no_latch = false;

  ut_d(block->page.in_page_hash = FALSE);
  ut_d(block->page.in_zip_hash = FALSE);
  ut_d(block->page.in_flush_list = FALSE);
  ut_d(block->page.in_free_list = FALSE);
  ut_d(block->page.in_LRU_list = FALSE);
  ut_d(block->in_unzip_LRU_list = FALSE);
  ut_d(block->in_withdraw_list = FALSE);

  page_zip_des_init(&block->page.zip);

  mutex_create(LATCH_ID_BUF_BLOCK_MUTEX, &block->mutex);

#if defined PFS_SKIP_BUFFER_MUTEX_RWLOCK || defined PFS_GROUP_BUFFER_SYNC
  /* If PFS_SKIP_BUFFER_MUTEX_RWLOCK is defined, skip registration
  of buffer block rwlock with performance schema.

  If PFS_GROUP_BUFFER_SYNC is defined, skip the registration
  since buffer block rwlock will be registered later in
  pfs_register_buffer_block(). */

  rw_lock_create(PFS_NOT_INSTRUMENTED, &block->lock, SYNC_LEVEL_VARYING);

  ut_d(rw_lock_create(PFS_NOT_INSTRUMENTED, &block->debug_latch,
                      SYNC_NO_ORDER_CHECK));

#else /* PFS_SKIP_BUFFER_MUTEX_RWLOCK || PFS_GROUP_BUFFER_SYNC */

  rw_lock_create(buf_block_lock_key, &block->lock, SYNC_LEVEL_VARYING);

  ut_d(rw_lock_create(buf_block_debug_latch_key, &block->debug_latch,
                      SYNC_NO_ORDER_CHECK));

#endif /* PFS_SKIP_BUFFER_MUTEX_RWLOCK || PFS_GROUP_BUFFER_SYNC */

  block->lock.is_block_lock = 1;

  ut_ad(rw_lock_validate(&(block->lock)));
}

