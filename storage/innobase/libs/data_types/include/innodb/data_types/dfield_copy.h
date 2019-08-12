#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Copies a data field to another.
@param[out]	field1	field to copy to
@param[in]	field2	field to copy from */
void dfield_copy(dfield_t *field1, const dfield_t *field2);
