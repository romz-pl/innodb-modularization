#pragma once

#include <innodb/univ/univ.h>

/** Determine if the field is not NULL and not having default value
after instant ADD COLUMN
@param[in]	len	length of a field
@return	true if not NULL and not having default value */
bool rec_field_not_null_not_add_col_def(ulint len);
