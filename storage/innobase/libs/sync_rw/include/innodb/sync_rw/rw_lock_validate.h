#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

/** Checks that the rw-lock has been initialized and that there are no
 simultaneous shared and exclusive locks.
 @return true */
bool rw_lock_validate(const rw_lock_t *lock); /*!< in: rw-lock */

#endif                                        /* UNIV_DEBUG */
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_LIBRARY */
