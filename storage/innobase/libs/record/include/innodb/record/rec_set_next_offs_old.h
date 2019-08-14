#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to set the next record offset field of an
old-style record.
@param[in]	rec	old-style physical record
@param[in]	next	offset of the next record */
void rec_set_next_offs_old(rec_t *rec, ulint next);
