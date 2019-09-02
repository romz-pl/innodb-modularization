#pragma once

#include <innodb/univ/univ.h>

/** Resize the lock hash table.
@param[in]	n_cells	number of slots in lock hash table */
void lock_sys_resize(ulint n_cells);
