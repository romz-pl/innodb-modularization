#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_LIBRARY

    #ifdef UNIV_DEBUG
        #define mutex_validate(M) (M)
    #endif /* UNIV_DEBUG */

#else /* UNIV_LIBRARY */

    #ifndef UNIV_HOTBACKUP

        #ifdef UNIV_DEBUG
            /**
            Checks that the mutex has been initialized. */
            #define mutex_validate(M) (M)->validate()
        #else
            #define mutex_validate(M) /* No op */
        #endif                    /* UNIV_DEBUG */

    #endif /* !UNIV_HOTBACKUP */

#endif /* UNIV_LIBRARY */
