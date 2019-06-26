#include <innodb/string/ut_strlcpy.h>
#include <algorithm>
#include <string.h>

/** Copies up to size - 1 characters from the NUL-terminated string src to
 dst, NUL-terminating the result. Returns strlen(src), so truncation
 occurred if the return value >= size.
 @return strlen(src) */
ulint ut_strlcpy(char *dst,       /*!< in: destination buffer */
                 const char *src, /*!< in: source buffer */
                 ulint size)      /*!< in: size of destination buffer */
{
  ulint src_size = strlen(src);

  if (size != 0) {
    ulint n = std::min(src_size, size - 1);

    memcpy(dst, src, n);
    dst[n] = '\0';
  }

  return (src_size);
}
