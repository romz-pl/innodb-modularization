#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Initialize the virtual field data in a dtuple_t
@param[in,out]		vrow	dtuple contains the virtual fields */
void dtuple_init_v_fld(const dtuple_t *vrow);
