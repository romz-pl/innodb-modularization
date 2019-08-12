#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;
struct dtype_t;

/** Sets the type struct of SQL data field.
@param[in]	field	SQL data field
@param[in]	type	pointer to data type struct */
void dfield_set_type(dfield_t *field, const dtype_t *type);
