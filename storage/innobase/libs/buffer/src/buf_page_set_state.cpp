#include <innodb/buffer/buf_page_set_state.h>

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buf_page/buf_page_state.h>
#include <innodb/buf_page/buf_page_get_state.h>

/** Sets the state of a block.
@param[in,out]	bpage	pointer to control block
@param[in]	state	state */
void buf_page_set_state(buf_page_t *bpage, buf_page_state state) {
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG
  enum buf_page_state old_state = buf_page_get_state(bpage);
  buf_pool_t *buf_pool = buf_pool_from_bpage(bpage);

  switch (old_state) {
    case BUF_BLOCK_POOL_WATCH:
      ut_error;
      break;
    case BUF_BLOCK_ZIP_PAGE:
      ut_a(state == BUF_BLOCK_ZIP_DIRTY);
      break;
    case BUF_BLOCK_ZIP_DIRTY:
      ut_a(state == BUF_BLOCK_ZIP_PAGE);
      ut_a(mutex_own(buf_page_get_mutex(bpage)));
      ut_a(buf_flush_list_mutex_own(buf_pool));
      ut_a(bpage->in_flush_list);
      break;
    case BUF_BLOCK_NOT_USED:
      ut_a(state == BUF_BLOCK_READY_FOR_USE);
      ut_a(buf_page_is_private(bpage, false, false));
      break;
    case BUF_BLOCK_READY_FOR_USE:
      ut_a(state == BUF_BLOCK_MEMORY || state == BUF_BLOCK_FILE_PAGE ||
           state == BUF_BLOCK_NOT_USED);
      ut_a(buf_page_is_private(bpage, state == BUF_BLOCK_FILE_PAGE,
                               state == BUF_BLOCK_NOT_USED));
      break;
    case BUF_BLOCK_MEMORY:
      ut_a(state == BUF_BLOCK_NOT_USED);
      ut_a(buf_page_is_private(bpage, false, true));
      break;
    case BUF_BLOCK_FILE_PAGE:
      ut_a(state == BUF_BLOCK_NOT_USED || state == BUF_BLOCK_REMOVE_HASH);
      if (state == BUF_BLOCK_REMOVE_HASH) {
        ut_a(!bpage->in_page_hash);
        ut_a(!bpage->in_zip_hash);
        ut_a(!bpage->in_LRU_list);
        ut_a(!bpage->in_free_list);
        ut_a(mutex_own(buf_page_get_mutex(bpage)));
        ut_a(mutex_own(&buf_pool->LRU_list_mutex));
        ut_a(buf_page_hash_lock_held_x(buf_pool, bpage));
      }
      break;
    case BUF_BLOCK_REMOVE_HASH:
      ut_a(state == BUF_BLOCK_MEMORY);
      break;
  }
#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */
  bpage->state = state;
  ut_ad(buf_page_get_state(bpage) == state);
}
