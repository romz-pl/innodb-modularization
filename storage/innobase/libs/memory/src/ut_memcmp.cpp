#include <innodb/memory/ut_memcmp.h>
#include <string.h>

/** Wrapper for memcmp(3).  Compare memory areas.
@param[in]	str1	first memory block to compare
@param[in]	str2	second memory block to compare
@param[in]	n	number of bytes to compare
@return negative, 0, or positive if str1 is smaller, equal,
                or greater than str2, respectively. */
int ut_memcmp(const void *str1, const void *str2, ulint n) {
  return (memcmp(str1, str2, n));
}
