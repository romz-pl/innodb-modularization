#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_create_func.h>
#include <innodb/memory/macros.h>

#ifdef UNIV_DEBUG

#define mem_heap_create_at(N, file_name, line) \
  mem_heap_create_func(N, file_name, line, MEM_HEAP_DYNAMIC)

#else /* UNIV_DEBUG */

#define mem_heap_create_at(N, file_name, line) \
  mem_heap_create_func(N, MEM_HEAP_DYNAMIC)

#endif /* UNIV_DEBUG */

