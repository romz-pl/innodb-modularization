#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

/** Frees a debug info struct. */
void rw_lock_debug_free(rw_lock_debug_t *info);

#endif /* UNIV_DEBUG */
