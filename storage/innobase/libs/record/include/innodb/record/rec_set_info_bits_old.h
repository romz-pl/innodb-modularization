#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to set the info bits of a record.
@param[in]	rec	old-style physical record
@param[in]	bits	info bits */
void rec_set_info_bits_old(rec_t *rec, ulint bits);
