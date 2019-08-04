#pragma once

#include <innodb/univ/univ.h>

/** Returns the minimum size of a data type.
@param[in]	mtype		main type
@param[in]	prtype		precise type
@param[in]	len		length
@param[in]	mbminmaxlen	minimum and maximum length of a multibyte
                                character, in bytes
@return minimum size */
ulint dtype_get_min_size_low(ulint mtype, ulint prtype, ulint len,
                             ulint mbminmaxlen);
