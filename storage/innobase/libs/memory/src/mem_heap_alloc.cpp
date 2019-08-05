#include <innodb/memory/mem_heap_alloc.h>

#include <innodb/memory/mem_block_validate.h>
#include <innodb/memory/mem_block_get_len.h>
#include <innodb/memory/mem_block_get_free.h>
#include <innodb/memory/mem_heap_add_block.h>
#include <innodb/memory/mem_block_get_free.h>
#include <innodb/memory/macros.h>
#include <innodb/memory/mem_block_set_free.h>
#include <innodb/memory_check/memory_check.h>

/** Allocates n bytes of memory from a memory heap.
@param[in]	heap	memory heap
@param[in]	n	number of bytes; if the heap is allowed to grow into
the buffer pool, this must be <= MEM_MAX_ALLOC_IN_BUF
@return allocated storage, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
void *mem_heap_alloc(mem_heap_t *heap, ulint n) {
  mem_block_t *block;
  byte *buf;
  ulint free;

  ut_d(mem_block_validate(heap));

  block = UT_LIST_GET_LAST(heap->base);

  ut_ad(!(block->type & MEM_HEAP_BUFFER) || (n <= MEM_MAX_ALLOC_IN_BUF));

  /* Check if there is enough space in block. If not, create a new
  block to the heap */

  if (mem_block_get_len(block) <
      mem_block_get_free(block) + MEM_SPACE_NEEDED(n)) {
    block = mem_heap_add_block(heap, n);

    if (block == NULL) {
      return (NULL);
    }
  }

  free = mem_block_get_free(block);

  buf = (byte *)block + free + MEM_NO_MANS_LAND;

  mem_block_set_free(block, free + MEM_SPACE_NEEDED(n));

  ut_ad(mem_block_get_len(block) >= mem_block_get_free(block));

  UNIV_MEM_ALLOC(buf - MEM_NO_MANS_LAND, MEM_SPACE_NEEDED(n));

#ifdef UNIV_DEBUG
  byte *start_no_mans_land = buf - MEM_NO_MANS_LAND;
  byte *end_no_mans_land =
      start_no_mans_land + MEM_SPACE_NEEDED(n) - MEM_NO_MANS_LAND;
  memset(start_no_mans_land, MEM_NO_MANS_LAND_BEFORE_BYTE, MEM_NO_MANS_LAND);
  UNIV_MEM_FREE(start_no_mans_land, MEM_NO_MANS_LAND);

  memset(end_no_mans_land, MEM_NO_MANS_LAND_AFTER_BYTE, MEM_NO_MANS_LAND);
  UNIV_MEM_FREE(end_no_mans_land, MEM_NO_MANS_LAND);
#endif

  return (buf);
}

