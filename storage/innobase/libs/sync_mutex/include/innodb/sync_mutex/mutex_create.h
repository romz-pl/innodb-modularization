#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/mutex_init.h>

#ifdef UNIV_LIBRARY

    #define mutex_create(I, M) (void)M

#else
    #ifndef UNIV_HOTBACKUP
        #define mutex_create(I, M) mutex_init((M), (I), __FILE__, __LINE__)
    #endif


#endif
