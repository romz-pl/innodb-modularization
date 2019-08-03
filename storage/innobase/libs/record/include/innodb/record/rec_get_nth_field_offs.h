#pragma once

#include <innodb/univ/univ.h>

/** The following function is used to get an offset to the nth data field in a
record.
@param[in]	offsets	array returned by rec_get_offsets()
@param[in]	n	index of the field
@param[out]	len	length of the field; UNIV_SQL_NULL if SQL null;
                        UNIV_SQL_ADD_COL_DEFAULT if it's default value and no
value inlined
@return offset from the origin of rec */
ulint rec_get_nth_field_offs(const ulint *offsets, ulint n, ulint *len);

