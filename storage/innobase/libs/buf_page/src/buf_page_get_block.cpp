#include <innodb/buf_page/buf_page_get_block.h>

#include <innodb/buf_block/buf_block_t.h>
#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buf_page/buf_page_get_state.h>

/** Gets the buf_block_t handle of a buffered file block if an uncompressed
page frame exists, or NULL. page frame exists, or NULL. The caller must hold
either the appropriate hash lock in any mode, either the LRU list mutex. Note:
even though bpage is not declared a const we don't update its value. It is safe
to make this pure.
@param[in]	bpage	control block, or NULL
@return control block, or NULL */
buf_block_t *buf_page_get_block(buf_page_t *bpage) {
  if (bpage != NULL) {
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG
    buf_pool_t *buf_pool = buf_pool_from_bpage(bpage);
    ut_ad(buf_page_hash_lock_held_s_or_x(buf_pool, bpage) ||
          mutex_own(&buf_pool->LRU_list_mutex));
#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */
    ut_ad(buf_page_in_file(bpage));

    if (buf_page_get_state(bpage) == BUF_BLOCK_FILE_PAGE) {
      return ((buf_block_t *)bpage);
    }
  }

  return (NULL);
}
