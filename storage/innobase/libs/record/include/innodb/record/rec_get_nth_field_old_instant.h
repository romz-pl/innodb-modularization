#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;


/** Gets the value of the specified field in the record in old style.
This is only used for record from instant index, which is clustered
index and has some instantly added columns.
@param[in]	rec	physical record
@param[in]	n	index of the field
@param[in]	index	clustered index where the record resides
@param[out]	len	length of the field, UNIV_SQL if SQL null
@return value of the field, could be either pointer to rec or default value */
const byte *rec_get_nth_field_old_instant(const rec_t *rec, uint16_t n,
                                          const dict_index_t *index,
                                          ulint *len);
