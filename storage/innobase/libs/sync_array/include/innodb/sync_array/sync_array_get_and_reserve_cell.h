#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_cell_t.h>

/** Get an instance of the sync wait array and reserve a wait array cell
 in the instance for waiting for an object. The event of the cell is
 reset to nonsignalled state.
 If reserving cell of the instance fails, try to get another new
 instance until we can reserve an empty cell of it.
 @return the sync array reserved, never NULL. */
sync_array_t *sync_array_get_and_reserve_cell(
    void *object,       /*!< in: pointer to the object to wait for */
    ulint type,         /*!< in: lock request type */
    const char *file,   /*!< in: file where requested */
    ulint line,         /*!< in: line where requested */
    sync_cell_t **cell); /*!< out: the cell reserved, never NULL */
