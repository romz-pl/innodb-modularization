#include <innodb/memory/mem_heap_strdup.h>

#include <innodb/memory/mem_heap_dup.h>

#include <string.h>

/** Duplicates a NUL-terminated string, allocated from a memory heap.
@param[in]	heap	memory heap where string is allocated
@param[in]	str)	string to be copied
@return own: a copy of the string */
char *mem_heap_strdup(mem_heap_t *heap, const char *str) {
  return (static_cast<char *>(mem_heap_dup(heap, str, strlen(str) + 1)));
}
