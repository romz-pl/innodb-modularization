#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Catenates a compressed ulint to mlog.
@param[in]	mtr	mtr
@param[in]	val	value to write */
void mlog_catenate_ulint_compressed(mtr_t *mtr, ulint val);
