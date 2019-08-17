#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_buf_t.h>
#include <innodb/log_types/mlog_id_t.h>

struct mtr_t;

/** Catenates 1 - 4 bytes to the mtr log. The value is not compressed.
@param[in,out]	dyn_buf	buffer to write
@param[in]	val	value to write
@param[in]	type	type of value to write */
void mlog_catenate_ulint(mtr_buf_t *dyn_buf, ulint val, mlog_id_t type);


/** Catenates 1 - 4 bytes to the mtr log.
@param[in]	mtr	mtr
@param[in]	val	value to write
@param[in]	type	MLOG_1BYTE, MLOG_2BYTES, MLOG_4BYTES */
void mlog_catenate_ulint(mtr_t *mtr, ulint val, mlog_id_t type);
