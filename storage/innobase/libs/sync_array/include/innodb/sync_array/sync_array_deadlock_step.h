#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <innodb/sync_array/sync_cell_t.h>

/** Recursion step for deadlock detection.
 @return true if deadlock detected */
ibool sync_array_deadlock_step(
    sync_array_t *arr,     /*!< in: wait array; NOTE! the caller must
                           own the mutex to array */
    sync_cell_t *start,    /*!< in: cell where recursive search
                           started */
    os_thread_id_t thread, /*!< in: thread to look at */
    ulint pass,            /*!< in: pass value */
    ulint depth);           /*!< in: recursion depth */

#endif
