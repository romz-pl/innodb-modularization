#pragma once

#include <innodb/univ/univ.h>


/** Like ut_strlcpy, but if src doesn't fit in dst completely, copies the last
 (size - 1) bytes of src, not the first.
 @return strlen(src) */
ulint ut_strlcpy_rev(char *dst,       /*!< in: destination buffer */
                     const char *src, /*!< in: source buffer */
                     ulint size);     /*!< in: size of destination buffer */

