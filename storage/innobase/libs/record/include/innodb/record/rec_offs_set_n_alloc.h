#pragma once

#include <innodb/univ/univ.h>

/** The following function sets the number of allocated elements
 for an array of offsets. */
void rec_offs_set_n_alloc(ulint *offsets, /*!< out: array for rec_get_offsets(),
                                          must be allocated */
                          ulint n_alloc);  /*!< in: number of elements */
