#include <innodb/memory/mem_heap_validate.h>

#ifdef UNIV_DEBUG
/** Validates the contents of a memory heap.
Checks a memory heap for consistency, prints the contents if any error
is detected. A fatal error is logged if an error is detected.
@param[in]	heap	Memory heap to validate. */
void mem_heap_validate(const mem_heap_t *heap) {
  ulint size = 0;

  for (const mem_block_t *block = heap; block != NULL;
       block = UT_LIST_GET_NEXT(list, block)) {
    mem_block_validate(block);

    switch (block->type) {
      case MEM_HEAP_DYNAMIC:
        break;
      case MEM_HEAP_BUFFER:
      case MEM_HEAP_BUFFER | MEM_HEAP_BTR_SEARCH:
        ut_ad(block->len <= UNIV_PAGE_SIZE);
        break;
      default:
        ut_error;
    }

    size += block->len;
  }

  ut_ad(size == heap->total_size);
}
#endif /* UNIV_DEBUG */
