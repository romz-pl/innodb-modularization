#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Sets number of fields used in a tuple. Normally this is set in
 dtuple_create, but if you want later to set it smaller, you can use this. */
void dtuple_set_n_fields(dtuple_t *tuple, /*!< in: tuple */
                         ulint n_fields); /*!< in: number of fields */
