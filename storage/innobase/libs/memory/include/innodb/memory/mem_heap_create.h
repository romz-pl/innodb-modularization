#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_create_func.h>
#include <innodb/memory/macros.h>


#ifdef UNIV_DEBUG

/** Macro for memory heap creation.
@param[in]	size		Desired start block size. */
#define mem_heap_create(size) \
  mem_heap_create_func((size), __FILE__, __LINE__, MEM_HEAP_DYNAMIC)

#else

/** Macro for memory heap creation.
@param[in]	size		Desired start block size. */
#define mem_heap_create(size) mem_heap_create_func((size), MEM_HEAP_DYNAMIC)

#endif
