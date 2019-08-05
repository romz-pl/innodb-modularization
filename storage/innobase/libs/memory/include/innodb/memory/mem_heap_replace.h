#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Allocate a new chunk of memory from a memory heap, possibly discarding the
topmost element. If the memory chunk specified with (top, top_sz) is the
topmost element, then it will be discarded, otherwise it will be left untouched
and this function will be equivallent to mem_heap_alloc().
@param[in,out]	heap	memory heap
@param[in]	top	chunk to discard if possible
@param[in]	top_sz	size of top in bytes
@param[in]	new_sz	desired size of the new chunk
@return allocated storage, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
void *mem_heap_replace(mem_heap_t *heap, const void *top, ulint top_sz,
                       ulint new_sz);
