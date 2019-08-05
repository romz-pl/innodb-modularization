#include <innodb/memory/ut_strlcpy_rev.h>

#include <cstring>
#include <utility>

/** Like ut_strlcpy, but if src doesn't fit in dst completely, copies the last
 (size - 1) bytes of src, not the first.
 @return strlen(src) */
ulint ut_strlcpy_rev(char *dst,       /*!< in: destination buffer */
                     const char *src, /*!< in: source buffer */
                     ulint size)      /*!< in: size of destination buffer */
{
  ulint src_size = strlen(src);

  if (size != 0) {
    ulint n = std::min(src_size, size - 1);

    memcpy(dst, src + src_size - n, n + 1);
  }

  return (src_size);
}
