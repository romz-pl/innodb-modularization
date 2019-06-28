#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/UT_NEW_ARRAY.h>

/** Allocate and account 'n_elements' objects of type 'type'.
Use this macro to allocate memory within InnoDB instead of 'new[]' and
instead of UT_NEW_ARRAY() when it is not feasible to create a dedicated key.
@param[in]	type		type of objects being created
@param[in]	n_elements	number of objects to create
@return pointer to the first allocated object or NULL */
#define UT_NEW_ARRAY_NOKEY(type, n_elements) \
  UT_NEW_ARRAY(type, n_elements, PSI_NOT_INSTRUMENTED)

#else

#define UT_NEW_ARRAY_NOKEY(type, n_elements) \
  ::new (std::nothrow) type[n_elements]

#endif
