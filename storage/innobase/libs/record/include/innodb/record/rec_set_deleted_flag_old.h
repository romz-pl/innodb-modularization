#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to set the deleted bit.
@param[in]	rec		old-style physical record
@param[in]	flag		nonzero if delete marked */
void rec_set_deleted_flag_old(rec_t *rec, ulint flag);
