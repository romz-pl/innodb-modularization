#include <innodb/buffer/buf_flush_ready_for_replace.h>

#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buffer/buf_page_get_mutex.h>
#include <innodb/buffer/buf_page_in_file.h>
#include <innodb/logger/fatal.h>
#include <innodb/buffer/buf_page_get_io_fix.h>

/** Returns TRUE if the file page block is immediately suitable for replacement,
i.e., the transition FILE_PAGE => NOT_USED allowed. The caller must hold the
LRU list and block mutexes.
@param[in]	bpage	buffer control block, must be buf_page_in_file() and
                        in the LRU list
@return true if can replace immediately */
ibool buf_flush_ready_for_replace(buf_page_t *bpage) {
#ifdef UNIV_DEBUG
  buf_pool_t *buf_pool = buf_pool_from_bpage(bpage);
  ut_ad(mutex_own(&buf_pool->LRU_list_mutex));
#endif /* UNIV_DEBUG */
  ut_ad(mutex_own(buf_page_get_mutex(bpage)));
  ut_ad(bpage->in_LRU_list);

  if (buf_page_in_file(bpage)) {
    return (bpage->oldest_modification == 0 && bpage->buf_fix_count == 0 &&
            buf_page_get_io_fix(bpage) == BUF_IO_NONE);
  }

  ib::fatal(ER_IB_MSG_123) << "Buffer block " << bpage << " state "
                           << bpage->state << " in the LRU list!";

  return (FALSE);
}
