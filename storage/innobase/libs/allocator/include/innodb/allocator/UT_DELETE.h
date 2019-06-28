#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/ut_delete.h>

/** Destroy, deallocate and trace the deallocation of an object created by
UT_NEW() or UT_NEW_NOKEY().
We can't instantiate ut_allocator without having the type of the object, thus
we redirect this to a template function. */
#define UT_DELETE(ptr) ut_delete(ptr)

#else

#define UT_DELETE(ptr) ::delete ptr

#endif
