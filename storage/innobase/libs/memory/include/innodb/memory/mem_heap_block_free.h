#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Frees a block from a memory heap. */
void mem_heap_block_free(mem_heap_t *heap,    /*!< in: heap */
                         mem_block_t *block); /*!< in: block to free */
