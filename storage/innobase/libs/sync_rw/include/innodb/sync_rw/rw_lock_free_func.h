#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Calling this function is obligatory only if the memory buffer containing
 the rw-lock is freed. Removes an rw-lock object from the global list. The
 rw-lock is checked to be in the non-locked state. */
void rw_lock_free_func(rw_lock_t *lock); /*!< in/out: rw-lock */

#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
