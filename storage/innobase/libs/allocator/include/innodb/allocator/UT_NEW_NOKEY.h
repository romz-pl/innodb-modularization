#pragma once

#include <innodb/allocator/UT_NEW_NOKEY.h>

#ifdef UNIV_PFS_MEMORY

#include <innodb/allocator/UT_NEW.h>

/** Allocate, trace the allocation and construct an object.
Use this macro instead of 'new' within InnoDB and instead of UT_NEW()
when creating a dedicated memory key is not feasible.
For example: instead of
        Foo*	f = new Foo(args);
use:
        Foo*	f = UT_NEW_NOKEY(Foo(args));
Upon failure to allocate the memory, this macro may return NULL. It
will not throw exceptions. After successful allocation the returned
pointer must be passed to UT_DELETE() when no longer needed.
@param[in]	expr	any expression that could follow "new"
@return pointer to the created object or NULL */
#define UT_NEW_NOKEY(expr) UT_NEW(expr, PSI_NOT_INSTRUMENTED)

#else

#define UT_NEW_NOKEY(expr) ::new (std::nothrow) expr

#endif
