#pragma once

#include <innodb/univ/univ.h>

struct dtype_t;

/** Gets the precise data type.
 @return precise data type */
ulint dtype_get_prtype(const dtype_t *type); /*!< in: data type */
