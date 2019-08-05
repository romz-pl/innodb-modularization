#include <innodb/memory/mem_heap_free.h>

#include <innodb/memory/mem_block_validate.h>
#include <innodb/memory/mem_heap_free_block_free.h>
#include <innodb/memory/mem_heap_block_free.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/lst/lst.h>

/** Frees the space occupied by a memory heap.
NOTE: Use the corresponding macro instead of this function.
@param[in]	heap	Heap to be freed */
void mem_heap_free(mem_heap_t *heap) {
  mem_block_t *block;
  mem_block_t *prev_block;

  ut_d(mem_block_validate(heap));

  block = UT_LIST_GET_LAST(heap->base);

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY
  if (heap->free_block) {
    mem_heap_free_block_free(heap);
  }
#endif /* !UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */

  while (block != NULL) {
    /* Store the contents of info before freeing current block
    (it is erased in freeing) */

    prev_block = UT_LIST_GET_PREV(list, block);

    mem_heap_block_free(heap, block);

    block = prev_block;
  }
}
