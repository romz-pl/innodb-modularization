#include <innodb/buffer/buf_page_set_old.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buf_page/buf_page_in_file.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Flag a block old.
@param[in]	bpage	control block
@param[in]	old	old */
void buf_page_set_old(buf_page_t *bpage, ibool old) {
#ifdef UNIV_DEBUG
  buf_pool_t *buf_pool = buf_pool_from_bpage(bpage);
#endif /* UNIV_DEBUG */
  ut_a(buf_page_in_file(bpage));
  ut_ad(mutex_own(&buf_pool->LRU_list_mutex));
  ut_ad(bpage->in_LRU_list);

#ifdef UNIV_LRU_DEBUG
  ut_a((buf_pool->LRU_old_len == 0) == (buf_pool->LRU_old == NULL));
  /* If a block is flagged "old", the LRU_old list must exist. */
  ut_a(!old || buf_pool->LRU_old);

  if (UT_LIST_GET_PREV(LRU, bpage) && UT_LIST_GET_NEXT(LRU, bpage)) {
    const buf_page_t *prev = UT_LIST_GET_PREV(LRU, bpage);
    const buf_page_t *next = UT_LIST_GET_NEXT(LRU, bpage);
    if (prev->old == next->old) {
      ut_a(prev->old == old);
    } else {
      ut_a(!prev->old);
      ut_a(buf_pool->LRU_old == (old ? bpage : next));
    }
  }
#endif /* UNIV_LRU_DEBUG */

  bpage->old = old;
}

#endif
