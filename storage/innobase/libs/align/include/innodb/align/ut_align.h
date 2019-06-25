#pragma once

#include <innodb/univ/univ.h>

/** The following function rounds up a pointer to the nearest aligned address.
@param[in]	ptr		pointer
@param[in]	align_no	align by this number
@return aligned pointer */
void *ut_align(const void *ptr, ulint align_no);
