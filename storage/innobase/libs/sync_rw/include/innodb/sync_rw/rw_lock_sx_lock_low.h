#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Low-level function for acquiring an sx lock.
 @return false if did not succeed, true if success. */
ibool rw_lock_sx_lock_low(
    rw_lock_t *lock,       /*!< in: pointer to rw-lock */
    ulint pass,            /*!< in: pass value; != 0, if the lock will
                           be passed to another thread to unlock */
    const char *file_name, /*!< in: file name where lock requested */
    ulint line);           /*!< in: line where requested */

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
