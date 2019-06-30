#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_cell_t.h>

/** Reports info of a wait array cell. */
void sync_array_cell_print(FILE *file, /*!< in: file where to print */
                                  sync_cell_t *cell); /*!< in: sync cell */
