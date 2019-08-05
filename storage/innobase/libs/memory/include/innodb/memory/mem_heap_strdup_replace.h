#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Allocate a new chunk of memory from a memory heap, possibly discarding the
topmost element and then copy the specified string to it. If the memory chunk
specified with (top, top_sz) is the topmost element, then it will be discarded,
otherwise it will be left untouched and this function will be equivalent to
mem_heap_strdup().
@param[in,out]	heap	memory heap
@param[in]	top	chunk to discard if possible
@param[in]	top_sz	size of top in bytes
@param[in]	str	new data to duplicate
@return allocated string, NULL if did not succeed (only possible for
MEM_HEAP_BTR_SEARCH type heaps) */
char *mem_heap_strdup_replace(mem_heap_t *heap, const void *top, ulint top_sz,
                              const char *str);
