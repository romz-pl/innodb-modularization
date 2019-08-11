#include <innodb/buf_block/buf_block_unfix.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/atomic/atomic.h>
#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buf_block/buf_block_t.h>

/** Decrements the bufferfix count.
@param[in,out]	bpage	block to bufferunfix
@return	the remaining buffer-fix count */
ulint buf_block_unfix(buf_page_t *bpage) {
  ib_uint32_t count;
  count = os_atomic_decrement_uint32(&bpage->buf_fix_count, 1);
  ut_ad(count + 1 != 0);
  return (count);
}

/** Decrements the bufferfix count.
@param[in,out]	block	block to bufferunfix
@return the remaining buffer-fix count */
ulint buf_block_unfix(buf_block_t *block) {
  return (buf_block_unfix(&block->page));
}

#endif
