#include <innodb/sync_rw/rw_lock_debug_free.h>

#ifdef UNIV_DEBUG

/** Frees a debug info struct. */
void rw_lock_debug_free(rw_lock_debug_t *info) {
    ut_free(info);
}

#endif /* UNIV_DEBUG */
