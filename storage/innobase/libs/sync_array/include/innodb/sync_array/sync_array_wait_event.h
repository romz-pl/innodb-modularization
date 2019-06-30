#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_array_t.h>

/** This function should be called when a thread starts to wait on
 a wait array cell. In the debug version this function checks
 if the wait for a semaphore will result in a deadlock, in which
 case prints info and asserts. */
void sync_array_wait_event(
    sync_array_t *arr,  /*!< in: wait array */
    sync_cell_t *&cell); /*!< in: index of the reserved cell */
