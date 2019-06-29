#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_LIBRARY

    #define mutex_enter_nowait(M) (void)M

#else

    #ifndef UNIV_HOTBACKUP
        #define mutex_enter_nowait(M) (M)->trylock(__FILE__, __LINE__)
    #endif

#endif
