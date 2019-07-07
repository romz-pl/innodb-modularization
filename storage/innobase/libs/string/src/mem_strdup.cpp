#include <innodb/string/mem_strdup.h>

#include <innodb/allocator/ut_malloc_nokey.h>

#include <string.h>

/** Duplicates a NUL-terminated string.
 @return own: a copy of the string, must be deallocated with ut_free */
char *mem_strdup(const char *str) /*!< in: string to be copied */
{
  ulint len = strlen(str) + 1;
  return (static_cast<char *>(memcpy(ut_malloc_nokey(len), str, len)));
}
