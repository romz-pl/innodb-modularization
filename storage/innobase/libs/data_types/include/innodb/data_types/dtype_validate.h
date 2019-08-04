#pragma once

#include <innodb/univ/univ.h>

struct dtype_t;

/** Validates a data type structure.
 @return true if ok */
ibool dtype_validate(const dtype_t *type); /*!< in: type struct to validate */
