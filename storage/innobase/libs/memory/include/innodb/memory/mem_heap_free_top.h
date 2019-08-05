#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Frees the topmost element in a memory heap.
@param[in]	heap	memory heap
@param[in]	n	size of the topmost element
The size of the element must be given. */
void mem_heap_free_top(mem_heap_t *heap, ulint n);
