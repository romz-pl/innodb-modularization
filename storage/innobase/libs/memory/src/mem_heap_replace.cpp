#include <innodb/memory/mem_heap_replace.h>

#include <innodb/memory/mem_heap_is_top.h>
#include <innodb/memory/mem_heap_free_top.h>
#include <innodb/memory/mem_heap_alloc.h>

/** Allocate a new chunk of memory from a memory heap, possibly discarding
 the topmost element. If the memory chunk specified with (top, top_sz)
 is the topmost element, then it will be discarded, otherwise it will
 be left untouched and this function will be equivallent to
 mem_heap_alloc().
 @return allocated storage, NULL if did not succeed (only possible for
 MEM_HEAP_BTR_SEARCH type heaps) */
void *mem_heap_replace(mem_heap_t *heap, /*!< in/out: memory heap */
                       const void *top, /*!< in: chunk to discard if possible */
                       ulint top_sz,    /*!< in: size of top in bytes */
                       ulint new_sz) /*!< in: desired size of the new chunk */
{
  if (mem_heap_is_top(heap, top, top_sz)) {
    mem_heap_free_top(heap, top_sz);
  }

  return (mem_heap_alloc(heap, new_sz));
}
