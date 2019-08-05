#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Adds a new block to a memory heap.
 @param[in]	heap	memory heap
 @param[in]	n	number of bytes needed
 @return created block, NULL if did not succeed (only possible for
 MEM_HEAP_BTR_SEARCH type heaps) */
mem_block_t *mem_heap_add_block(mem_heap_t *heap, ulint n);

