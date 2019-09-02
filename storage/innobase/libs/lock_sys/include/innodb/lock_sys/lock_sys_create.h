#pragma once

#include <innodb/univ/univ.h>

/** Creates the lock system at database start. */
void lock_sys_create(
    ulint n_cells); /*!< in: number of slots in lock hash table */
