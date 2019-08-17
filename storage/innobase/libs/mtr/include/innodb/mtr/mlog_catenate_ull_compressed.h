#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Catenates a compressed 64-bit integer to mlog.
@param[in]	mtr	mtr
@param[in]	val	value to write */
void mlog_catenate_ull_compressed(mtr_t *mtr, ib_uint64_t val);
