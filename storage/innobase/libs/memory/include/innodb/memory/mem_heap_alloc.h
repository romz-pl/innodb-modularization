#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Allocates n bytes of memory from a memory heap.
@param[in]	heap	memory heap
@param[in]	n	number of bytes; if the heap is allowed to grow into
the buffer pool, this must be <= MEM_MAX_ALLOC_IN_BUF
@return allocated storage, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
void *mem_heap_alloc(mem_heap_t *heap, ulint n);

