#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** The following function returns the sum of data lengths of a tuple. The space
occupied by the field structs or the tuple struct is not counted.
@param[in]	tuple	typed data tuple
@param[in]	comp	nonzero=ROW_FORMAT=COMPACT
@return sum of data lens */
ulint dtuple_get_data_size(const dtuple_t *tuple, ulint comp);
