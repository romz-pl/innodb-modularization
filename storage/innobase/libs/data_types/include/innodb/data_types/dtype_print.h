#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

struct dtype_t;

/** Print a data type structure.
@param[in]	type	data type */
void dtype_print(const dtype_t *type);

#endif /* UNIV_DEBUG */
