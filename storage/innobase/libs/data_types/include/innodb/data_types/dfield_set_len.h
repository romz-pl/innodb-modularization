#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Sets length in a field.
@param[in]	field	field
@param[in]	len	length or UNIV_SQL_NULL */
void dfield_set_len(dfield_t *field, ulint len);
