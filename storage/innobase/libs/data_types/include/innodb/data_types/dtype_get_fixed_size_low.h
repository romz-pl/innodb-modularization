#pragma once

#include <innodb/univ/univ.h>

/** Returns the size of a fixed size data type, 0 if not a fixed size type.
@param[in]	mtype		main type
@param[in]	prtype		precise type
@param[in]	len		length
@param[in]	mbminmaxlen	minimum and maximum length of a multibyte
                                character, in bytes
@param[in]	comp		nonzero=ROW_FORMAT=COMPACT
@return fixed size, or 0 */
ulint dtype_get_fixed_size_low(ulint mtype, ulint prtype, ulint len,
                               ulint mbminmaxlen, ulint comp);
