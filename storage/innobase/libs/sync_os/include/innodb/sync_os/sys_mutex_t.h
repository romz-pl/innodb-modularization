#pragma once

#include <innodb/univ/univ.h>

/** Native mutex */

#ifdef _WIN32

    typedef CRITICAL_SECTION sys_mutex_t;

#else

    #include <pthread.h>
    typedef pthread_mutex_t sys_mutex_t;

#endif /* _WIN32 */
