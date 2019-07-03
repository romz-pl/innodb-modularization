#include <innodb/io/os_create_block_cache.h>

#include <innodb/io/block_cache.h>
#include <innodb/assert/assert.h>
#include <innodb/allocator/UT_NEW_NOKEY.h>
#include <innodb/allocator/ut_malloc_nokey.h>

/** Creates and initializes block_cache. Creates array of MAX_BLOCKS
and allocates the memory in each block to hold BUFFER_BLOCK_SIZE
of data.

This function is called by InnoDB during srv_start().
It is also called by MEB while applying the redo logs on TDE tablespaces,
the "Blocks" allocated in this block_cache are used to hold the decrypted
page data. */
void os_create_block_cache() {
  ut_a(block_cache == NULL);

  block_cache = UT_NEW_NOKEY(Blocks(MAX_BLOCKS));

  for (Blocks::iterator it = block_cache->begin(); it != block_cache->end();
       ++it) {
    ut_a(it->m_in_use == 0);
    ut_a(it->m_ptr == NULL);

    /* Allocate double of max page size memory, since
    compress could generate more bytes than orgininal
    data. */
    it->m_ptr = static_cast<byte *>(ut_malloc_nokey(BUFFER_BLOCK_SIZE));

    ut_a(it->m_ptr != NULL);
  }
}
