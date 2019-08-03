#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>


#ifdef UNIV_DEBUG

/** Check if the given two record offsets are identical.
@param[in]  offsets1  field offsets of a record
@param[in]  offsets2  field offsets of a record
@return true if they are identical, false otherwise. */
bool rec_offs_cmp(ulint *offsets1, ulint *offsets2);

#endif
