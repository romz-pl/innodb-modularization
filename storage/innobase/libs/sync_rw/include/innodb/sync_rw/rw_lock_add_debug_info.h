#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

/** Inserts the debug information for an rw-lock. */
void rw_lock_add_debug_info(
    rw_lock_t *lock,       /*!< in: rw-lock */
    ulint pass,            /*!< in: pass value */
    ulint lock_type,       /*!< in: lock type */
    const char *file_name, /*!< in: file where requested */
    ulint line);           /*!< in: line where requested */
/** Removes a debug information struct for an rw-lock. */
void rw_lock_remove_debug_info(rw_lock_t *lock,  /*!< in: rw-lock */
                               ulint pass,       /*!< in: pass value */
                               ulint lock_type); /*!< in: lock type */

#endif  /* UNIV_DEBUG */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
