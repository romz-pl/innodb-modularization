#include <innodb/memory/mem_heap_zalloc.h>

#include <innodb/memory/mem_heap_alloc.h>
#include <innodb/assert/assert.h>

#include <string.h>

/** Allocates and zero-fills n bytes of memory from a memory heap.
@param[in]	heap	memory heap
@param[in]	n	number of bytes; if the heap is allowed to grow into
the buffer pool, this must be <= MEM_MAX_ALLOC_IN_BUF
@return allocated, zero-filled storage */
void *mem_heap_zalloc(mem_heap_t *heap, ulint n) {
  ut_ad(heap);
  ut_ad(!(heap->type & MEM_HEAP_BTR_SEARCH));
  return (memset(mem_heap_alloc(heap, n), 0, n));
}
