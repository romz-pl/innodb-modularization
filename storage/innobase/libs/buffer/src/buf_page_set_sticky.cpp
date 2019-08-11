#include <innodb/buffer/buf_page_set_sticky.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buffer/buf_page_get_io_fix.h>
#include <innodb/buffer/buf_page_get_mutex.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Makes a block sticky. A sticky block implies that even after we release
the buf_pool->LRU_list_mutex and the block->mutex:
* it cannot be removed from the flush_list
* the block descriptor cannot be relocated
* it cannot be removed from the LRU list
Note that:
* the block can still change its position in the LRU list
* the next and previous pointers can change.
@param[in,out]	bpage	control block */
void buf_page_set_sticky(buf_page_t *bpage) {
#ifdef UNIV_DEBUG
  buf_pool_t *buf_pool = buf_pool_from_bpage(bpage);
  ut_ad(mutex_own(&buf_pool->LRU_list_mutex));
#endif /* UNIV_DEBUG */

  ut_ad(mutex_own(buf_page_get_mutex(bpage)));
  ut_ad(buf_page_get_io_fix(bpage) == BUF_IO_NONE);
  ut_ad(bpage->in_LRU_list);

  bpage->io_fix = BUF_IO_PIN;
}


#endif
