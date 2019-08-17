#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Closes a buffer opened to mlog.
@param[in]	mtr	mtr
@param[in]	ptr	buffer space from ptr up was not used */
void mlog_close(mtr_t *mtr, byte *ptr);
