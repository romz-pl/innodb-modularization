#include <innodb/memory/mem_heap_get_heap_top.h>

#include <innodb/memory/mem_block_info_t.h>
#include <innodb/memory/macros.h>
#include <innodb/memory/mem_block_validate.h>
#include <innodb/memory/mem_block_get_free.h>

/** Returns a pointer to the heap top.
@param[in]	heap	memory heap
@return pointer to the heap top */
byte *mem_heap_get_heap_top(mem_heap_t *heap) {
  mem_block_t *block;
  byte *buf;

  ut_d(mem_block_validate(heap));

  block = UT_LIST_GET_LAST(heap->base);

  buf = (byte *)block + mem_block_get_free(block);

  return (buf);
}
