#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Copies a data tuple's virtaul fields to another. This is a shallow copy;
@param[in,out]	d_tuple		destination tuple
@param[in]	s_tuple		source tuple */
void dtuple_copy_v_fields(dtuple_t *d_tuple, const dtuple_t *s_tuple);
