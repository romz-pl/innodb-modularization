#include <innodb/buffer/buf_page_release_zip.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_block_t.h>
#include <innodb/buffer/buf_block_unfix.h>
#include <innodb/buf_page/buf_page_get_state.h>
#include <innodb/buf_page/buf_page_t.h>
#include <innodb/error/ut_error.h>

/** Releases a compressed-only page acquired with buf_page_get_zip(). */
void buf_page_release_zip(buf_page_t *bpage) /*!< in: buffer block */
{
  ut_ad(bpage);
  ut_a(bpage->buf_fix_count > 0);

  switch (buf_page_get_state(bpage)) {
    case BUF_BLOCK_FILE_PAGE:
#ifdef UNIV_DEBUG
    {
      /* No debug latch is acquired if block belongs to system
      temporary. Debug latch is not of much help if access to block
      is single threaded. */
      buf_block_t *block = reinterpret_cast<buf_block_t *>(bpage);
      if (!fsp_is_system_temporary(block->page.id.space())) {
        rw_lock_s_unlock(&block->debug_latch);
      }
    }
#endif /* UNIV_DEBUG */
       /* Fall through */

    case BUF_BLOCK_ZIP_PAGE:
    case BUF_BLOCK_ZIP_DIRTY:
      buf_block_unfix(reinterpret_cast<buf_block_t *>(bpage));
      return;

    case BUF_BLOCK_POOL_WATCH:
    case BUF_BLOCK_NOT_USED:
    case BUF_BLOCK_READY_FOR_USE:
    case BUF_BLOCK_MEMORY:
    case BUF_BLOCK_REMOVE_HASH:
      break;
  }

  ut_error;
}

#endif
