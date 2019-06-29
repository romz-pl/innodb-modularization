#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_mutex/srv_spin_wait_delay.h>
#include <innodb/sync_mutex/srv_n_spin_wait_rounds.h>

#ifdef UNIV_LIBRARY

    #define mutex_enter(M) (void)M

#else
    #ifndef UNIV_HOTBACKUP


        #define mutex_enter(M) \
          (M)->enter(srv_n_spin_wait_rounds, srv_spin_wait_delay, __FILE__, __LINE__)
    #endif

#endif

