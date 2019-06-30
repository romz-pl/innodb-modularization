#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

/** Function for the next writer to call. Waits for readers to exit.
 The caller must have already decremented lock_word by X_LOCK_DECR. */
void rw_lock_x_lock_wait_func(
    rw_lock_t *lock, /*!< in: pointer to rw-lock */
#ifdef UNIV_DEBUG
    ulint pass, /*!< in: pass value; != 0, if the lock will
                be passed to another thread to unlock */
#endif
    lint threshold,        /*!< in: threshold to wait for */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line);            /*!< in: line where requested */
