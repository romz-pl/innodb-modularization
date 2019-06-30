#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <innodb/sync_array/sync_cell_t.h>

/** This function is called only in the debug version. Detects a deadlock
 of one or more threads because of waits of semaphores.
 @return true if deadlock detected */
bool sync_array_detect_deadlock(
    sync_array_t *arr,  /*!< in: wait array; NOTE! the caller must
                        own the mutex to array */
    sync_cell_t *start, /*!< in: cell where recursive search started */
    sync_cell_t *cell,  /*!< in: cell to search */
    ulint depth);       /*!< in: recursion depth */


#endif
