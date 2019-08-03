#pragma once

#include <innodb/univ/univ.h>

/** Mark the nth field as externally stored.
@param[in]	offsets		array returned by rec_get_offsets()
@param[in]	n		nth field */
void rec_offs_make_nth_extern(ulint *offsets, const ulint n);
