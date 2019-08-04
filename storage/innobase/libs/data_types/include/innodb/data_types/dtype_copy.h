#pragma once

#include <innodb/univ/univ.h>

struct dtype_t;

/** Copies a data type structure.
@param[in]	type1	type struct to copy to
@param[in]	type2	type struct to copy from */
void dtype_copy(dtype_t *type1, const dtype_t *type2);
