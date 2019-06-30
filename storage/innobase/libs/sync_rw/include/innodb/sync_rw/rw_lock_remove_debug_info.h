#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

/** Removes a debug information struct for an rw-lock. */
void rw_lock_remove_debug_info(rw_lock_t *lock, /*!< in: rw-lock */
                               ulint pass,      /*!< in: pass value */
                               ulint lock_type); /*!< in: lock type */


#endif
