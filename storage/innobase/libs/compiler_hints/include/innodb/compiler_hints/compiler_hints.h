#pragma once

#include <innodb/univ/univ.h>

//
// Some macros to improve branch prediction and reduce cache misses
//

#if defined(COMPILER_HINTS) && defined(__GNUC__)

    //
    // Tell the compiler that 'expr' probably evaluates to 'constant'.
    //
    #define UNIV_EXPECT(expr, constant) __builtin_expect(expr, constant)

    //
    // Tell the compiler that a pointer is likely to be NULL
    //
    #define UNIV_LIKELY_NULL(ptr) __builtin_expect((ulint)ptr, 0)

    //
    // Minimize cache-miss latency by moving data at addr into a cache before it is read.
    //
    #define UNIV_PREFETCH_R(addr) __builtin_prefetch(addr, 0, 3)

    //
    // Minimize cache-miss latency by moving data at addr into a cache before
    // it is read or written.
    #define UNIV_PREFETCH_RW(addr) __builtin_prefetch(addr, 1, 3)


#elif (defined(__SUNPRO_C) || defined(__SUNPRO_CC))

    //
    // Sun Studio includes sun_prefetch.h as of version 5.9
    //
    #include <sun_prefetch.h>

    #define UNIV_EXPECT(expr, value) (expr)
    #define UNIV_LIKELY_NULL(expr) (expr)

    #if defined(COMPILER_HINTS)
        //# define UNIV_PREFETCH_R(addr) sun_prefetch_read_many((void*) addr)
        #define UNIV_PREFETCH_R(addr) ((void)0)
        #define UNIV_PREFETCH_RW(addr) sun_prefetch_write_many(addr)
    #else
        #define UNIV_PREFETCH_R(addr) ((void)0)
        #define UNIV_PREFETCH_RW(addr) ((void)0)
    #endif /* COMPILER_HINTS */

#elif defined __WIN__ && defined COMPILER_HINTS

    #include <xmmintrin.h>

    #define UNIV_EXPECT(expr, value) (expr)
    #define UNIV_LIKELY_NULL(expr) (expr)
    // __MM_HINT_T0 - (temporal data)
    // prefetch data into all levels of the cache hierarchy.
    #define UNIV_PREFETCH_R(addr) _mm_prefetch((char *)addr, _MM_HINT_T0)
    #define UNIV_PREFETCH_RW(addr) _mm_prefetch((char *)addr, _MM_HINT_T0)

#else

    //
    // Dummy versions of the macros
    //

    #define UNIV_EXPECT(expr, value) (expr)
    #define UNIV_LIKELY_NULL(expr) (expr)
    #define UNIV_PREFETCH_R(addr) ((void)0)
    #define UNIV_PREFETCH_RW(addr) ((void)0)
#endif

//
// Tell the compiler that cond is likely to hold
//
#define UNIV_LIKELY(cond) UNIV_EXPECT(cond, TRUE)


//
// Tell the compiler that cond is unlikely to hold
//
#define UNIV_UNLIKELY(cond) UNIV_EXPECT(cond, FALSE)
