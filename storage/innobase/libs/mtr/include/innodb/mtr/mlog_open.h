#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Opens a buffer to mlog. It must be closed with mlog_close.
@param[in,out]	mtr	mtr
@param[in]	size	buffer size in bytes; MUST be smaller than
                        DYN_ARRAY_DATA_SIZE!
@return buffer, NULL if log mode MTR_LOG_NONE */
byte *mlog_open(mtr_t *mtr, ulint size);
