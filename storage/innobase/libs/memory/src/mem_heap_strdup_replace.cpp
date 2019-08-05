#include <innodb/memory/mem_heap_strdup_replace.h>

#include <innodb/memory/mem_heap_dup_replace.h>

#include <string.h>

/** Allocate a new chunk of memory from a memory heap, possibly discarding
 the topmost element and then copy the specified string to it. If the memory
 chunk specified with (top, top_sz) is the topmost element, then it will be
 discarded, otherwise it will be left untouched and this function will be
 equivallent to mem_heap_strdup().
 @return allocated string, NULL if did not succeed (only possible for
 MEM_HEAP_BTR_SEARCH type heaps) */
char *mem_heap_strdup_replace(
    mem_heap_t *heap, /*!< in/out: memory heap */
    const void *top,  /*!< in: chunk to discard if possible */
    ulint top_sz,     /*!< in: size of top in bytes */
    const char *str)  /*!< in: new data to duplicate */
{
  return (reinterpret_cast<char *>(
      mem_heap_dup_replace(heap, top, top_sz, str, strlen(str) + 1)));
}
