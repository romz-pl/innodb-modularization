#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/univ/rec_t.h>

struct mtr_t;

/** Write a 32-bit field in a data dictionary record.
@param[in,out]	rec	record to update
@param[in]	i	index of the field to update
@param[in]	val	value to write
@param[in,out]	mtr	mini-transaction */

void page_rec_write_field(rec_t *rec, ulint i, ulint val, mtr_t *mtr);

#endif /* !UNIV_HOTBACKUP */
