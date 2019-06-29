#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_LIBRARY

    #define mutex_exit(M) (void)M

#else
    #ifndef UNIV_HOTBACKUP
        #define mutex_exit(M) (M)->exit()
    #endif

#endif
