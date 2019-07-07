#include <innodb/buffer/buf_block_fix.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_page_t.h>
#include <innodb/buffer/buf_block_t.h>
#include <innodb/assert/assert.h>
#include <innodb/atomic/atomic.h>

/** Increments the bufferfix count.
@param[in,out]	bpage	block to bufferfix
@return the count */
ulint buf_block_fix(buf_page_t *bpage) {
  ib_uint32_t count;
  count = os_atomic_increment_uint32(&bpage->buf_fix_count, 1);
  ut_ad(count > 0);
  return (count);
}

/** Increments the bufferfix count.
@param[in,out]	block	block to bufferfix
@return the count */
ulint buf_block_fix(buf_block_t *block) {
  return (buf_block_fix(&block->page));
}

#endif
