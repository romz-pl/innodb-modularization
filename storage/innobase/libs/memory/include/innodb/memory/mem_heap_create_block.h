#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_create_block_func.h>

#ifdef UNIV_DEBUG

#define mem_heap_create_block(heap, n, type, file_name, line) \
  mem_heap_create_block_func(heap, n, file_name, line, type)

#else /* UNIV_DEBUG */

#define mem_heap_create_block(heap, n, type, file_name, line) \
  mem_heap_create_block_func(heap, n, type)


#endif /* UNIV_DEBUG */
