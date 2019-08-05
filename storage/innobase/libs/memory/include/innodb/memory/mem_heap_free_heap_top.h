#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Frees the space in a memory heap exceeding the pointer given.
The pointer must have been acquired from mem_heap_get_heap_top.
The first memory block of the heap is not freed.
@param[in]	heap		heap from which to free
@param[in]	old_top		pointer to old top of heap */
void mem_heap_free_heap_top(mem_heap_t *heap, byte *old_top);
