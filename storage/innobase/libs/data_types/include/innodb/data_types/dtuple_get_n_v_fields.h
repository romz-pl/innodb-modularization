#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Gets number of virtual fields in a data tuple.
@param[in]	tuple	dtuple to check
@return number of fields */
ulint dtuple_get_n_v_fields(const dtuple_t *tuple);
