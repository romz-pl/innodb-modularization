#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to set the instant bit.
@param[in,out]	rec	new-style physical record
@param[in]	flag	set the bit to this flag */
void rec_set_instant_flag_new(rec_t *rec, bool flag);
