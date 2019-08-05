#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_create_func.h>


#ifdef UNIV_DEBUG


/** Macro for memory heap creation.
@param[in]	size		Desired start block size.
@param[in]	type		Heap type */
#define mem_heap_create_typed(size, type) \
  mem_heap_create_func((size), __FILE__, __LINE__, (type))

#else /* UNIV_DEBUG */


/** Macro for memory heap creation.
@param[in]	size		Desired start block size.
@param[in]	type		Heap type */
#define mem_heap_create_typed(size, type) mem_heap_create_func((size), (type))

#endif /* UNIV_DEBUG */
