#include <innodb/memory/mem_heap_empty.h>

#include <innodb/memory/mem_heap_free_block_free.h>
#include <innodb/memory/mem_block_get_start.h>
#include <innodb/memory/mem_heap_free_heap_top.h>

/** Empties a memory heap.
The first memory block of the heap is not freed.
@param[in]	heap	heap to empty */
void mem_heap_empty(mem_heap_t *heap) {
  mem_heap_free_heap_top(heap, (byte *)heap + mem_block_get_start(heap));
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY
  if (heap->free_block) {
    mem_heap_free_block_free(heap);
  }
#endif /* !UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */
}
