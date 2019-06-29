#pragma once

#include <innodb/univ/univ.h>

#ifdef _WIN32

    /** Native condition variable. */
    typedef CONDITION_VARIABLE os_cond_t;

#else

    #include <pthread.h>
    /** Native condition variable. */
    typedef pthread_cond_t os_cond_t;

#endif /* _WIN32 */
