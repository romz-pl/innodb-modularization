#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Sets pointer to the data and length in a field.
@param[in]	field	field
@param[in]	mbr	data */
void dfield_write_mbr(dfield_t *field, const double *mbr);
