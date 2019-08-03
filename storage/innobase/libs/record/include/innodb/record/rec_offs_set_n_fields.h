#pragma once

#include <innodb/univ/univ.h>

/** The following function sets the number of fields in offsets. */
void rec_offs_set_n_fields(ulint *offsets, /*!< in/out: array returned by
                                           rec_get_offsets() */
                           ulint n_fields); /*!< in: number of fields */
