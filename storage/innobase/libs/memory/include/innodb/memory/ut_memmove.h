#pragma once

#include <innodb/univ/univ.h>

/** Wrapper for memmove(3).  Copy memory area when the source and
target are overlapping.
@param[in,out]	dest	Move to
@param[in]	src	Move from
@param[in]	n	number of bytes to move
@return dest */
void *ut_memmove(void *dest, const void *src, ulint n);
