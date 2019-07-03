#include <innodb/allocator/mem_strdupl.h>

#include <innodb/allocator/ut_malloc_nokey.h>

#include <string.h>

/** Makes a NUL-terminated copy of a nonterminated string.
 @return own: a copy of the string, must be deallocated with ut_free */
char *mem_strdupl(const char *str, /*!< in: string to be copied */
                  ulint len)       /*!< in: length of str, in bytes */
{
  char *s = static_cast<char *>(ut_malloc_nokey(len + 1));
  s[len] = 0;
  if (len > 0) {
    memcpy(s, str, len);
  }
  return s;
}
