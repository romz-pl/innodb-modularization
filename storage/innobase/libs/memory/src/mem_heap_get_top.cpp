#include <innodb/memory/mem_heap_get_top.h>

#include <innodb/memory/mem_block_validate.h>
#include <innodb/memory/mem_block_info_t.h>
#include <innodb/memory/mem_block_get_free.h>
#include <innodb/memory/macros.h>

/** Returns a pointer to the topmost element in a memory heap.
The size of the element must be given.
@param[in]	heap	memory heap
@param[in]	n	size of the topmost element
@return pointer to the topmost element */
void *mem_heap_get_top(mem_heap_t *heap, ulint n) {
  mem_block_t *block;
  byte *buf;

  ut_d(mem_block_validate(heap));

  block = UT_LIST_GET_LAST(heap->base);

  buf = (byte *)block + mem_block_get_free(block) - MEM_SPACE_NEEDED(n) +
        MEM_NO_MANS_LAND;

  return ((void *)buf);
}
