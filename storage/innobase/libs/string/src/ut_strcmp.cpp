#include <innodb/string/ut_strcmp.h>
#include <string.h>

/** Wrapper for strcmp(3).  Compare NUL-terminated strings.
@param[in]	str1	first string to compare
@param[in]	str2	second string to compare
@return negative, 0, or positive if str1 is smaller, equal,
                or greater than str2, respectively. */
int ut_strcmp(const char *str1, const char *str2) {
  return (strcmp(str1, str2));
}
