#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_delete_array.h>

/** Destroy, deallocate and trace the deallocation of an array created by
UT_NEW_ARRAY() or UT_NEW_ARRAY_NOKEY().
We can't instantiate ut_allocator without having the type of the object, thus
we redirect this to a template function. */
#define UT_DELETE_ARRAY(ptr) ut_delete_array(ptr)

#else

#define UT_DELETE_ARRAY(ptr) ::delete[] ptr

#endif
