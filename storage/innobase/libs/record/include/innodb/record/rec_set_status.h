#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to set the status bits of a new-style record.
@param[in,out]	rec	physical record
@param[in]	bits	info bits */
void rec_set_status(rec_t *rec, ulint bits);
