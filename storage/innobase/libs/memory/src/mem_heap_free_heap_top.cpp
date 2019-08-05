#include <innodb/memory/mem_heap_free_heap_top.h>

#include <innodb/memory/mem_heap_validate.h>
#include <innodb/memory/mem_block_get_free.h>
#include <innodb/memory/mem_heap_block_free.h>
#include <innodb/memory/mem_block_get_start.h>
#include <innodb/memory/macros.h>
#include <innodb/memory/mem_block_set_free.h>
#include <innodb/memory_check/memory_check.h>

/** Frees the space in a memory heap exceeding the pointer given.
The pointer must have been acquired from mem_heap_get_heap_top.
The first memory block of the heap is not freed.
@param[in]	heap		heap from which to free
@param[in]	old_top		pointer to old top of heap */
void mem_heap_free_heap_top(mem_heap_t *heap, byte *old_top) {
  mem_block_t *block;
  mem_block_t *prev_block;

  ut_d(mem_heap_validate(heap));

  block = UT_LIST_GET_LAST(heap->base);

  while (block != NULL) {
    if (((byte *)block + mem_block_get_free(block) >= old_top) &&
        ((byte *)block <= old_top)) {
      /* Found the right block */

      break;
    }

    /* Store prev_block value before freeing the current block
    (the current block will be erased in freeing) */

    prev_block = UT_LIST_GET_PREV(list, block);

    mem_heap_block_free(heap, block);

    block = prev_block;
  }

  ut_ad(block);

  if (old_top == (byte *)block + mem_block_get_free(block)) {
    return;
  }

#ifdef UNIV_DEBUG
  validate_no_mans_land(old_top, MEM_NO_MANS_LAND_BEFORE_BYTE);
  validate_no_mans_land(
      (byte *)block + mem_block_get_free(block) - MEM_NO_MANS_LAND,
      MEM_NO_MANS_LAND_AFTER_BYTE);
#endif
  /* Set the free field of block */
  mem_block_set_free(block, old_top - (byte *)block);

  ut_ad(mem_block_get_start(block) <= mem_block_get_free(block));
  UNIV_MEM_FREE(old_top, (byte *)block + block->len - old_top);

  /* If free == start, we may free the block if it is not the first
  one */

  if ((heap != block) &&
      (mem_block_get_free(block) == mem_block_get_start(block))) {
    mem_heap_block_free(heap, block);
  }
}
