#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Frees the space occupied by a memory heap.
NOTE: Use the corresponding macro instead of this function.
@param[in]	heap	Heap to be freed */
void mem_heap_free(mem_heap_t *heap);
