#include <innodb/memory/mem_heap_dup_replace.h>

#include <innodb/memory/mem_heap_replace.h>

#include <string.h>

/** Allocate a new chunk of memory from a memory heap, possibly discarding
 the topmost element and then copy the specified data to it. If the memory
 chunk specified with (top, top_sz) is the topmost element, then it will be
 discarded, otherwise it will be left untouched and this function will be
 equivallent to mem_heap_dup().
 @return allocated storage, NULL if did not succeed (only possible for
 MEM_HEAP_BTR_SEARCH type heaps) */
void *mem_heap_dup_replace(
    mem_heap_t *heap, /*!< in/out: memory heap */
    const void *top,  /*!< in: chunk to discard if possible */
    ulint top_sz,     /*!< in: size of top in bytes */
    const void *data, /*!< in: new data to duplicate */
    ulint data_sz)    /*!< in: size of data in bytes */
{
  void *p = mem_heap_replace(heap, top, top_sz, data_sz);

  memcpy(p, data, data_sz);

  return (p);
}
