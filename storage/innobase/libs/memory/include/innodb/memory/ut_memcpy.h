#pragma once

#include <innodb/univ/univ.h>

/** Wrapper for memcpy(3).  Copy memory area when the source and
target are not overlapping.
@param[in,out]	dest	copy to
@param[in]	src	copy from
@param[in]	n	number of bytes to copy
@return dest */
void *ut_memcpy(void *dest, const void *src, ulint n);
