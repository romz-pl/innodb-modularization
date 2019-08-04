#pragma once

#include <innodb/univ/univ.h>

struct dtype_t;


/** Gets the SQL main data type.
 @return SQL main data type */
ulint dtype_get_mtype(const dtype_t *type); /*!< in: data type */
