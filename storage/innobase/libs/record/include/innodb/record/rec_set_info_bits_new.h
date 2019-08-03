#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to set the info bits of a record.
@param[in,out]	rec	new-style physical record
@param[in]	bits	info bits */
void rec_set_info_bits_new(rec_t *rec, ulint bits);
