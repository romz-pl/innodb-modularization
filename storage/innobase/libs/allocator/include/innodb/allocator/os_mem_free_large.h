#pragma once

#include <innodb/univ/univ.h>

/** Frees large pages memory.
@param[in]	ptr	pointer returned by os_mem_alloc_large()
@param[in]	size	size returned by os_mem_alloc_large() */
void os_mem_free_large(void *ptr, ulint size);
