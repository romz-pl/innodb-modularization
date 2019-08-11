#include <innodb/buf_block/buf_block_buf_fix_dec.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buf_block/buf_block_t.h>
#include <innodb/buf_block/buf_block_unfix.h>

/** Decrements the bufferfix count. */
void buf_block_buf_fix_dec(
    buf_block_t *block) /*!< in/out: block to bufferunfix */
{
  buf_block_unfix(block);

#ifdef UNIV_DEBUG
  /* No debug latch is acquired if block belongs to system temporary.
  Debug latch is not of much help if access to block is single
  threaded. */
  if (!fsp_is_system_temporary(block->page.id.space())) {
    rw_lock_s_unlock(&block->debug_latch);
  }
#endif /* UNIV_DEBUG */
}

#endif
