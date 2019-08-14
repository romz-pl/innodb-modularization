#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Set the number of fields for one new style leaf page record.
This is only needed for table after instant ADD COLUMN.
@param[in,out]	rec		leaf page record
@param[in]	n_fields	number of fields in the record
@return the length of the n_fields occupies */
uint8_t rec_set_n_fields(rec_t *rec, ulint n_fields);
