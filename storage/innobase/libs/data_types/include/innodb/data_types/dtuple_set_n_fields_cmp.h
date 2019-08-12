#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Gets number of fields used in record comparisons.
@param[in]	tuple		tuple
@param[in]	n_fields_cmp	number of fields used in comparisons in
                                rem0cmp */
void dtuple_set_n_fields_cmp(dtuple_t *tuple, ulint n_fields_cmp);
