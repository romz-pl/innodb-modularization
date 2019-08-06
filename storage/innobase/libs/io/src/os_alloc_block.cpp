#include <innodb/io/os_alloc_block.h>

#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/assert/assert.h>
#include <innodb/io/block_cache.h>
#include <innodb/thread/os_thread_yield.h>

#include <my_rdtsc.h>

/** Allocate a page for sync IO
@return pointer to page */
Block *os_alloc_block() {
  size_t pos;
  Blocks &blocks = *block_cache;
  size_t i = static_cast<size_t>(my_timer_cycles());
  const size_t size = blocks.size();
  ulint retry = 0;
  Block *block;

  DBUG_EXECUTE_IF("os_block_cache_busy", retry = MAX_BLOCKS * 3;);

  for (;;) {
    /* After go through the block cache for 3 times,
    allocate a new temporary block. */
    if (retry == MAX_BLOCKS * 3) {
      byte *ptr;

      ptr = static_cast<byte *>(
          ut_malloc_nokey(sizeof(*block) + BUFFER_BLOCK_SIZE));

      block = new (ptr) Block();
      block->m_ptr = static_cast<byte *>(ptr + sizeof(*block));
      block->m_in_use = 1;

      break;
    }

    pos = i++ % size;

    if (TAS(&blocks[pos].m_in_use, 1) == 0) {
      block = &blocks[pos];
      break;
    }

    os_thread_yield();

    ++retry;
  }

  ut_a(block->m_in_use != 0);

  return (block);
}
