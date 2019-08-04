#pragma once

#include <innodb/univ/univ.h>

struct dtype_t;

/** Sets a data type structure.
@param[in]	type	type struct to init
@param[in]	mtype	main data type
@param[in]	prtype	precise type
@param[in]	len	precision of type */
void dtype_set(dtype_t *type, ulint mtype, ulint prtype, ulint len);
