#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Get the number of fields for one new style leaf page record.
This is only needed for table after instant ADD COLUMN.
@param[in]	rec		leaf page record
@param[in]	extra_bytes	extra bytes of this record
@param[in,out]	length		length of number of fields
@return	number of fields */
uint32_t rec_get_n_fields_instant(const rec_t *rec, const ulint extra_bytes,
                                  uint16_t *length);
