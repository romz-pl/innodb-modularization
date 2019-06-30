#include <innodb/sync_rw/rw_lock_debug_create.h>

#ifdef UNIV_DEBUG

/** Creates a debug info struct.
 @return own: debug info struct */
rw_lock_debug_t *rw_lock_debug_create(void) {
  return ((rw_lock_debug_t *)ut_malloc_nokey(sizeof(rw_lock_debug_t)));
}

#endif /* UNIV_DEBUG */
