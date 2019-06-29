#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/mutex_destroy.h>

#ifdef UNIV_LIBRARY

    #define mutex_free(M) (void)M

#else
    #ifndef UNIV_HOTBACKUP
        #define mutex_free(M) mutex_destroy(M)
    #endif

#endif
