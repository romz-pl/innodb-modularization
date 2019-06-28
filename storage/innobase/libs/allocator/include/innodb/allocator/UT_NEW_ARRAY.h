#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_allocator.h>

/** Allocate and account 'n_elements' objects of type 'type'.
Use this macro to allocate memory within InnoDB instead of 'new[]'.
The returned pointer must be passed to UT_DELETE_ARRAY().
@param[in]	type		type of objects being created
@param[in]	n_elements	number of objects to create
@param[in]	key		performance schema memory tracing key
@return pointer to the first allocated object or NULL */
#define UT_NEW_ARRAY(type, n_elements, key) \
  ut_allocator<type>(key).new_array(n_elements, UT_NEW_THIS_FILE_PSI_KEY)

#else

#define UT_NEW_ARRAY(type, n_elements, key) \
  ::new (std::nothrow) type[n_elements]

#endif
