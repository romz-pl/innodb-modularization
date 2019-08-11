#include <innodb/buffer/buf_page_get_mutex.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_block_t.h>
#include <innodb/buffer/buf_page_get_state.h>
#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buffer/buf_pool_from_bpage.h>
#include <innodb/buffer/buf_pool_t.h>
#include <innodb/error/ut_error.h>

/** Gets the mutex of a block.
 @return pointer to mutex protecting bpage */
BPageMutex *buf_page_get_mutex(
    const buf_page_t *bpage) /*!< in: pointer to control block */
{
  buf_pool_t *buf_pool = buf_pool_from_bpage(bpage);

  switch (buf_page_get_state(bpage)) {
    case BUF_BLOCK_POOL_WATCH:
      ut_error;
    case BUF_BLOCK_ZIP_PAGE:
    case BUF_BLOCK_ZIP_DIRTY:
      return (&buf_pool->zip_mutex);
    default:
      return (&((buf_block_t *)bpage)->mutex);
  }
}

#endif
