#include <innodb/buffer/buf_block_get_frame.h>

#include <innodb/assert/assert.h>
#include <innodb/buf_block/buf_block_t.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG
/** Gets a pointer to the memory frame of a block.
 @return pointer to the frame */
buf_frame_t *buf_block_get_frame(
    const buf_block_t *block) /*!< in: pointer to the control block */
{
  ut_ad(block);

  switch (buf_block_get_state(block)) {
    case BUF_BLOCK_POOL_WATCH:
    case BUF_BLOCK_ZIP_PAGE:
    case BUF_BLOCK_ZIP_DIRTY:
    case BUF_BLOCK_NOT_USED:
      ut_error;
      break;
    case BUF_BLOCK_FILE_PAGE:
      ut_a(block->page.buf_fix_count > 0);
      /* fall through */
    case BUF_BLOCK_READY_FOR_USE:
    case BUF_BLOCK_MEMORY:
    case BUF_BLOCK_REMOVE_HASH:
      goto ok;
  }
  ut_error;
ok:
  return ((buf_frame_t *)block->frame);
}
#else /* UNIV_DEBUG */
buf_frame_t *buf_block_get_frame(const buf_block_t *block) {
    ut_ad(block);
    return (block->frame);
}
#endif /* UNIV_DEBUG */
#else  /* !UNIV_HOTBACKUP */
buf_frame_t *buf_block_get_frame(const buf_block_t *block) {
    ut_ad(block);
    return (block->frame);
}
#endif /* !UNIV_HOTBACKUP */
