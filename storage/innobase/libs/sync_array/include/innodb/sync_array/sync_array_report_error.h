#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <innodb/sync_array/sync_cell_t.h>

/**
Report an error to stderr.
@param lock		rw-lock instance
@param debug		rw-lock debug information
@param cell		thread context */
void sync_array_report_error(rw_lock_t *lock, rw_lock_debug_t *debug,
                                    sync_cell_t *cell);

#endif
