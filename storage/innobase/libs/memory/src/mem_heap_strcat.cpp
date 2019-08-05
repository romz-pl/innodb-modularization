#include <innodb/memory/mem_heap_strcat.h>

#include <innodb/memory/mem_heap_alloc.h>

#include <string.h>

/** Concatenate two strings and return the result, using a memory heap.
 @return own: the result */
char *mem_heap_strcat(
    mem_heap_t *heap, /*!< in: memory heap where string is allocated */
    const char *s1,   /*!< in: string 1 */
    const char *s2)   /*!< in: string 2 */
{
  char *s;
  ulint s1_len = strlen(s1);
  ulint s2_len = strlen(s2);

  s = static_cast<char *>(mem_heap_alloc(heap, s1_len + s2_len + 1));

  memcpy(s, s1, s1_len);
  memcpy(s + s1_len, s2, s2_len);

  s[s1_len + s2_len] = '\0';

  return (s);
}
