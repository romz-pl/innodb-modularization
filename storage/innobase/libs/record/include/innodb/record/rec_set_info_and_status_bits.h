#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to set the info and status bits of a record.
(Only compact records have status bits.)
@param[in,out]	rec	compact physical record
@param[in]	bits	info bits */
void rec_set_info_and_status_bits(rec_t *rec, ulint bits);
