#include <innodb/memory/mem_heap_strdupl.h>

#include <innodb/memory/mem_heap_alloc.h>

#include <string.h>

/** Makes a NUL-terminated copy of a nonterminated string,
 allocated from a memory heap.
 @return own: a copy of the string */
char *mem_heap_strdupl(
    mem_heap_t *heap, /*!< in: memory heap where string is allocated */
    const char *str,  /*!< in: string to be copied */
    ulint len)        /*!< in: length of str, in bytes */
{
  char *s = (char *)mem_heap_alloc(heap, len + 1);
  s[len] = 0;
  if (len > 0) {
    memcpy(s, str, len);
  }
  return s;
}
