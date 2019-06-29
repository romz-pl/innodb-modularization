#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/BlockSysMutex.h>
#include <innodb/sync_mutex/BlockFutexMutex.h>
#include <innodb/sync_mutex/BlockSyncArrayMutex.h>


#ifdef UNIV_LIBRARY

    typedef OSMutex ib_bpmutex_t;

#else

    #ifndef UNIV_HOTBACKUP
        #ifdef MUTEX_FUTEX
            typedef BlockFutexMutex ib_bpmutex_t;
        #elif defined(MUTEX_SYS)
            typedef BlockSysMutex ib_bpmutex_t;
        #elif defined(MUTEX_EVENT)
            typedef BlockSyncArrayMutex ib_bpmutex_t;
        #else
            #error "ib_bpmutex_t type is unknown"
        #endif
    #else
        #include "../meb/mutex.h"
        typedef meb::Mutex ib_bpmutex_t;
    #endif


#endif
