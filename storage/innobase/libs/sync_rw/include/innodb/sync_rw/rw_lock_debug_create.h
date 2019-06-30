#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

/** Creates a debug info struct. */
rw_lock_debug_t *rw_lock_debug_create(void);

#endif /* UNIV_DEBUG */
