#include <innodb/buffer/buf_block_buf_fix_inc_func.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_block_t.h>
#include <innodb/buffer/buf_block_fix.h>

/** Increments the bufferfix count. */
void buf_block_buf_fix_inc_func(
#ifdef UNIV_DEBUG
    const char *file,   /*!< in: file name */
    ulint line,         /*!< in: line */
#endif                  /* UNIV_DEBUG */
    buf_block_t *block) /*!< in/out: block to bufferfix */
{
#ifdef UNIV_DEBUG
  /* No debug latch is acquired if block belongs to system temporary.
  Debug latch is not of much help if access to block is single
  threaded. */
  if (!fsp_is_system_temporary(block->page.id.space())) {
    ibool ret;
    ret = rw_lock_s_lock_nowait(&block->debug_latch, file, line);
    ut_a(ret);
  }
#endif /* UNIV_DEBUG */

  buf_block_fix(block);
}

#endif
