#include <innodb/memory/mem_heap_get_size.h>

#include <innodb/memory/mem_block_validate.h>
#include <innodb/memory/mem_block_info_t.h>

/** Returns the space in bytes occupied by a memory heap. */
ulint mem_heap_get_size(mem_heap_t *heap) /*!< in: heap */
{
  ulint size = 0;

  ut_d(mem_block_validate(heap));

  size = heap->total_size;

#ifndef UNIV_LIBRARY
  if (heap->free_block) {
    size += UNIV_PAGE_SIZE;
  }
#endif /* !UNIV_LIBRARY */

  return (size);
}
