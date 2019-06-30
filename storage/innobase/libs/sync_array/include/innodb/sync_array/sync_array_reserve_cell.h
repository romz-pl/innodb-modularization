#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_array_t.h>

/** Reserves a wait array cell for waiting for an object.
 The event of the cell is reset to nonsignalled state.
 @return sync cell to wait on */
sync_cell_t *sync_array_reserve_cell(
    sync_array_t *arr, /*!< in: wait array */
    void *object,      /*!< in: pointer to the object to wait for */
    ulint type,        /*!< in: lock request type */
    const char *file,  /*!< in: file where requested */
    ulint line);        /*!< in: line where requested */
