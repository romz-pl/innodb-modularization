#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to retrieve the info bits of a temporary
record.
@param[in]	rec	physical record
@return	info bits */
ulint rec_get_info_bits_temp(const rec_t *rec);
