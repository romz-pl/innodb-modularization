#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Computes the number of externally stored fields in a data tuple.
 @return number of fields */
ulint dtuple_get_n_ext(const dtuple_t *tuple); /*!< in: tuple */
