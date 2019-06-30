#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_array_t.h>

/** Frees the cell. NOTE! sync_array_wait_event frees the cell
 automatically! */
void sync_array_free_cell(
    sync_array_t *arr,  /*!< in: wait array */
    sync_cell_t *&cell); /*!< in/out: the cell in the array */
