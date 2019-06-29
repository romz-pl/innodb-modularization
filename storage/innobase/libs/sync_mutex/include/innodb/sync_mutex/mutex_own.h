#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_LIBRARY
    #ifdef UNIV_DEBUG
        /* Since mutexes are disabled under UNIV_LIBRARY, the following is OK
        and necessary to suppress compiler warnings. */
        #define mutex_own(M) ((M) || false)
    #endif /* UNIV_DEBUG */

#else /* UNIV_LIBRARY */

    #ifndef UNIV_HOTBACKUP

        #ifdef UNIV_DEBUG

            /**
            Checks that the current thread owns the mutex. Works only
            in the debug version. */
            #define mutex_own(M) (M)->is_owned()
        #else
            #define mutex_own(M)      /* No op */
        #endif                    /* UNIV_DEBUG */
    #endif /* !UNIV_HOTBACKUP */

#endif /* UNIV_LIBRARY */
