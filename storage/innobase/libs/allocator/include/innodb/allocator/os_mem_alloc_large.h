#pragma once

#include <innodb/univ/univ.h>

/** Allocates large pages memory.
@param[in,out]	n	Number of bytes to allocate
@return allocated memory */
void *os_mem_alloc_large(ulint *n);
