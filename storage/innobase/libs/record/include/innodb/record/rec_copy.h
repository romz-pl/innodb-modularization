#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Copy a physical record to a buffer.
@param[in]	buf	buffer
@param[in]	rec	physical record
@param[in]	offsets	array returned by rec_get_offsets()
@return pointer to the origin of the copy */
rec_t *rec_copy(void *buf, const rec_t *rec, const ulint *offsets);
