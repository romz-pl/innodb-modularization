#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Empties a memory heap.
The first memory block of the heap is not freed.
@param[in]	heap		heap to empty */
void mem_heap_empty(mem_heap_t *heap);
