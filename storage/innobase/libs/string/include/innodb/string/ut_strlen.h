#pragma once

#include <innodb/univ/univ.h>

/** Wrapper for strlen(3).  Determine the length of a NUL-terminated string.
@param[in]	str	string
@return length of the string in bytes, excluding the terminating NUL */
ulint ut_strlen(const char *str);
