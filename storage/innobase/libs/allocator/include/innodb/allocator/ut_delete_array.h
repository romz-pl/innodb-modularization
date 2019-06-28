#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_allocator.h>

/** Destroy and account objects created by UT_NEW_ARRAY() or
UT_NEW_ARRAY_NOKEY().
@param[in,out]	ptr	pointer to the first object in the array */
template <typename T>
inline void ut_delete_array(T *ptr) {
  ut_allocator<T>().delete_array(ptr);
}

#else

// No code here!

#endif
