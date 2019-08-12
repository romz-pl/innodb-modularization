#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Sets pointer to the data and length in a field.
@param[in]	field	field
@param[in]	data	data
@param[in]	len	length or UNIV_SQL_NULL */
void dfield_set_data(dfield_t *field, const void *data, ulint len);
