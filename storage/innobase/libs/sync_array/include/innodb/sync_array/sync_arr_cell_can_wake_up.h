#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_cell_t.h>

/** Determines if we can wake up the thread waiting for a sempahore. */
bool sync_arr_cell_can_wake_up(
    sync_cell_t *cell); /*!< in: cell to search */
