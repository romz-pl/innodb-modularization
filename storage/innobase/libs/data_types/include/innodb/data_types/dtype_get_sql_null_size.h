#pragma once

#include <innodb/univ/univ.h>

struct dtype_t;

/** Returns the ROW_FORMAT=REDUNDANT stored SQL NULL size of a type.
For fixed length types it is the fixed length of the type, otherwise 0.
@param[in]	type	type struct
@param[in]	comp	nonzero=ROW_FORMAT=COMPACT
@return SQL null storage size in ROW_FORMAT=REDUNDANT */
ulint dtype_get_sql_null_size(const dtype_t *type, ulint comp);
