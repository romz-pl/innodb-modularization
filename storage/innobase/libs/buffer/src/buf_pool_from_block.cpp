#include <innodb/buffer/buf_pool_from_block.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_pool_from_bpage.h>
#include <innodb/buf_block/buf_block_t.h>

/** Returns the buffer pool instance given a block instance
 @return buf_pool */
buf_pool_t *buf_pool_from_block(const buf_block_t *block) /*!< in: block */
{
  return (buf_pool_from_bpage(&block->page));
}

#endif
