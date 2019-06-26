#pragma once

#include <innodb/univ/univ.h>

/** Wrapper for strcpy(3).  Copy a NUL-terminated string.
@param[in,out]	dest	Destination to copy to
@param[in]	src	Source to copy from
@return dest */
char *ut_strcpy(char *dest, const char *src);
