#include <innodb/buf_page/buf_page_release_latch.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_block/buf_block_t.h>
#include <innodb/sync_rw/rw_lock_s_unlock.h>
#include <innodb/sync_rw/rw_lock_sx_unlock.h>
#include <innodb/sync_rw/rw_lock_type_t.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>

/** Releases a latch, if specified. */
void buf_page_release_latch(buf_block_t *block, /*!< in: buffer block */
                            ulint rw_latch)     /*!< in: RW_S_LATCH, RW_X_LATCH,
                                                RW_NO_LATCH */
{
#ifdef UNIV_DEBUG
  /* No debug latch is acquired if block belongs to system
  temporary. Debug latch is not of much help if access to block
  is single threaded. */
  if (!fsp_is_system_temporary(block->page.id.space())) {
    rw_lock_s_unlock(&block->debug_latch);
  }
#endif /* UNIV_DEBUG */

  if (rw_latch == RW_S_LATCH) {
    rw_lock_s_unlock(&block->lock);
  } else if (rw_latch == RW_SX_LATCH) {
    rw_lock_sx_unlock(&block->lock);
  } else if (rw_latch == RW_X_LATCH) {
    rw_lock_x_unlock(&block->lock);
  }
}

#endif
