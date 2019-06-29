#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/OSTrackMutex.h>
#include <innodb/sync_mutex/FutexMutex.h>
#include <innodb/sync_mutex/SyncArrayMutex.h>
#include <innodb/sync_mutex/SysMutex.h>


#ifdef UNIV_LIBRARY

    #include <innodb/sync_os/OSMutex.h>
    typedef OSMutex ib_mutex_t;

#else

    #ifndef UNIV_HOTBACKUP
        #ifdef MUTEX_FUTEX
            typedef FutexMutex ib_mutex_t;
        #elif defined(MUTEX_SYS)
            typedef SysMutex ib_mutex_t;
        #elif defined(MUTEX_EVENT)
            typedef SyncArrayMutex ib_mutex_t;
        #else
            #error "ib_mutex_t type is unknown"
        #endif
    #else
        #include "../meb/mutex.h"
        typedef meb::Mutex ib_mutex_t;
    #endif

#endif
