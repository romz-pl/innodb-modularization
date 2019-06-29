#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

    #ifdef MUTEX_FUTEX
        /** The default mutex type. */
        #define MUTEX_TYPE "Uses futexes"
    #elif defined(MUTEX_SYS)
        #define MUTEX_TYPE "Uses system mutexes"
    #elif defined(MUTEX_EVENT)
        #define MUTEX_TYPE "Uses event mutexes"
    #else
        #error "MUTEX_TYPE type is unknown"
    #endif /* MUTEX_FUTEX */

#endif
#endif
