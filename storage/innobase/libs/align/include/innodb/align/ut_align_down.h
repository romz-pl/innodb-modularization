#pragma once

#include <innodb/univ/univ.h>

/** The following function rounds down a pointer to the nearest
 aligned address.
 @return aligned pointer */
void *ut_align_down(const void *ptr, /*!< in: pointer */
                    ulint align_no)  /*!< in: align by this number */
    MY_ATTRIBUTE((const));
