#pragma once

#include <innodb/univ/univ.h>

/** Returns the maximum size of a data type. Note: types in system tables may be
incomplete and return incorrect information.
@param[in]	mtype	main type
@param[in]	len	length
@return maximum size */
ulint dtype_get_max_size_low(ulint mtype, ulint len);
