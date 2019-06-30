#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_array_t.h>

sync_cell_t *sync_array_get_nth_cell(
    sync_array_t *arr, /*!< in: sync array */
    ulint n);           /*!< in: index */
