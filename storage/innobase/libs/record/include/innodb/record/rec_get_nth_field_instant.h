#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;

/** Gets the value of the specified field in the record.
This is only used when there is possibility that the record comes from the
clustered index, which has some instantly added columns.
@param[in]	rec	physical record
@param[in]	offsets	array returned by rec_get_offsets()
@param[in]	n	index of the field
@param[in]	index	clustered index where the record resides, or nullptr
                        if the record doesn't have instantly added columns
                        for sure
@param[out]	len	length of the field, UNIV_SQL_NULL if SQL null
@return	value of the field, could be either pointer to rec or default value */
const byte *rec_get_nth_field_instant(const rec_t *rec, const ulint *offsets,
                                      ulint n, const dict_index_t *index,
                                      ulint *len);


