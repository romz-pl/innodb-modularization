#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/** Creates, or rather, initializes an rw-lock object in a specified memory
 location (which must be appropriately aligned). The rw-lock is initialized
 to the non-locked state. Explicit freeing of the rw-lock with rw_lock_free
 is necessary only if the memory block containing it is freed. */
void rw_lock_create_func(
    rw_lock_t *lock, /*!< in: pointer to memory */
#ifdef UNIV_DEBUG
    latch_level_t level,     /*!< in: level */
    const char *cmutex_name, /*!< in: mutex name */
#endif                       /* UNIV_DEBUG */
    const char *cfile_name,  /*!< in: file name where created */
    ulint cline);            /*!< in: file line where created */


#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
