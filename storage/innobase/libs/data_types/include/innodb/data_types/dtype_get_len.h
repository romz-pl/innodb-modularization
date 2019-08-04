#pragma once

#include <innodb/univ/univ.h>

struct dtype_t;

/** Gets the type length.
 @return fixed length of the type, in bytes, or 0 if variable-length */
ulint dtype_get_len(const dtype_t *type); /*!< in: data type */
