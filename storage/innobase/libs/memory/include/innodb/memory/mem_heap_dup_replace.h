#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Allocate a new chunk of memory from a memory heap, possibly discarding the
topmost element and then copy the specified data to it. If the memory chunk
specified with (top, top_sz) is the topmost element, then it will be discarded,
otherwise it will be left untouched and this function will be equivalent to
mem_heap_dup().
@param[in,out]	heap	memory heap
@param[in]	top	chunk to discard if possible
@param[in]	top_sz	size of top in bytes
@param[in]	data	new data to duplicate
@param[in]	data_sz	size of data in bytes
@return allocated storage, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
void *mem_heap_dup_replace(mem_heap_t *heap, const void *top, ulint top_sz,
                           const void *data, ulint data_sz);
