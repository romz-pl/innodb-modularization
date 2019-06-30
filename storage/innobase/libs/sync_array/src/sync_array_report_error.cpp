#include <innodb/sync_array/sync_array_report_error.h>

#ifdef UNIV_DEBUG

/**
Report an error to stderr.
@param lock		rw-lock instance
@param debug		rw-lock debug information
@param cell		thread context */
void sync_array_report_error(rw_lock_t *lock, rw_lock_debug_t *debug,
                                    sync_cell_t *cell) {
  fprintf(stderr, "rw-lock %p ", (void *)lock);
  sync_array_cell_print(stderr, cell);
  rw_lock_debug_print(stderr, debug);
}


#endif /* UNIV_DEBUG */
