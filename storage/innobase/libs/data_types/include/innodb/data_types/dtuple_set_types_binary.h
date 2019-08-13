#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Sets types of fields binary in a tuple.
@param[in]	tuple	data tuple
@param[in]	n	number of fields to set */
void dtuple_set_types_binary(dtuple_t *tuple, ulint n);
