#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_allocator.h>

/** Destroy and account object created by UT_NEW() or UT_NEW_NOKEY().
@param[in,out]	ptr	pointer to the object */
template <typename T>
inline void ut_delete(T *ptr) {
  if (ptr == NULL) {
    return;
  }

  ut_allocator<T> allocator;

  allocator.destroy(ptr);
  allocator.deallocate(ptr);
}

#else

// No code here!

#endif
