#include <innodb/io/os_free_block.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/io/block_cache.h>

/** Free a page after sync IO
@param[in,out]	block		The block to free/release */
void os_free_block(Block *block) {
  ut_ad(block->m_in_use == 1);

  TAS(&block->m_in_use, 0);

  /* When this block is not in the block cache, and it's
  a temporary block, we need to free it directly. */
  if (std::less<Block *>()(block, &block_cache->front()) ||
      std::greater<Block *>()(block, &block_cache->back())) {
    ut_free(block);
  }
}
