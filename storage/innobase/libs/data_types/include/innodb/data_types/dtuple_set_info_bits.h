#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Sets info bits in a data tuple.
@param[in]	tuple		tuple
@param[in]	info_bits	info bits */
void dtuple_set_info_bits(dtuple_t *tuple, ulint info_bits);
