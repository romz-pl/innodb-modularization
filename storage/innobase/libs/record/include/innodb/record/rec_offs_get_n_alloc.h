#pragma once

#include <innodb/univ/univ.h>

/** The following function returns the number of allocated elements
 for an array of offsets.
 @return number of elements */
MY_ATTRIBUTE((warn_unused_result)) ulint rec_offs_get_n_alloc(
    const ulint *offsets); /*!< in: array for rec_get_offsets() */
