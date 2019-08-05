#include <innodb/memory/mem_heap_dup.h>

#include <innodb/memory/mem_heap_alloc.h>

#include <cstring>

/** Duplicate a block of data, allocated from a memory heap.
 @return own: a copy of the data */
void *mem_heap_dup(
    mem_heap_t *heap, /*!< in: memory heap where copy is allocated */
    const void *data, /*!< in: data to be copied */
    ulint len)        /*!< in: length of data, in bytes */
{
  return (memcpy(mem_heap_alloc(heap, len), data, len));
}
