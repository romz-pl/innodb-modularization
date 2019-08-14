#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to retrieve the info bits of a record.
@param[in]	rec	physical record
@param[in]	comp	nonzero=compact page format
@return info bits */
ulint rec_get_info_bits(const rec_t *rec, ulint comp);
