#pragma once

#include <innodb/univ/univ.h>

/** The following function computes the offset of a pointer from the nearest
 aligned address.
 @return distance from aligned pointer */
ulint ut_align_offset(const void *ptr, /*!< in: pointer */
                      ulint align_no)  /*!< in: align by this number */
    MY_ATTRIBUTE((const));
