#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_MEMORY

#include <cstddef>
#include <innodb/allocator/ut_allocator.h>

/** Allocate, trace the allocation and construct an object.
Use this macro instead of 'new' within InnoDB.
For example: instead of
        Foo*	f = new Foo(args);
use:
        Foo*	f = UT_NEW(Foo(args), mem_key_some);
Upon failure to allocate the memory, this macro may return NULL. It
will not throw exceptions. After successful allocation the returned
pointer must be passed to UT_DELETE() when no longer needed.
@param[in]	expr	any expression that could follow "new"
@param[in]	key	performance schema memory tracing key
@return pointer to the created object or NULL */
#define UT_NEW(expr, key)                                                \
  /* Placement new will return NULL and not attempt to construct an      \
  object if the passed in pointer is NULL, e.g. if allocate() has        \
  failed to allocate memory and has returned NULL. */                    \
  ::new (ut_allocator<byte>(key).allocate(sizeof expr, NULL, key, false, \
                                          false)) expr

#else

#define UT_NEW(expr, key) ::new (std::nothrow) expr

#endif
