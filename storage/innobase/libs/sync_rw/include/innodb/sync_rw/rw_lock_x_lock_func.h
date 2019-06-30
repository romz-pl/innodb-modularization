#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP


/** NOTE! Use the corresponding macro, not directly this function! Lock an
 rw-lock in exclusive mode for the current thread. If the rw-lock is locked
 in shared or exclusive mode, or there is an exclusive lock request waiting,
 the function spins a preset time (controlled by srv_n_spin_wait_rounds),
 waiting for the lock, before suspending the thread. If the same thread has an
 x-lock on the rw-lock, locking succeed, with the following exception: if pass
 != 0, only a single x-lock may be taken on the lock. NOTE: If the same thread
 has an s-lock, locking does not succeed! */
void rw_lock_x_lock_func(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    ulint pass,            /*!< in: pass value; != 0, if the lock will
                           be passed to another thread to unlock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line);           /*!< in: line where requested */

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
