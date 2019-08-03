#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to set the 1-byte offsets flag.
@param[in]	rec	physical record
@param[in]	flag	TRUE if 1byte form */
void rec_set_1byte_offs_flag(rec_t *rec, ibool flag);
