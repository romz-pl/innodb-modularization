#pragma once

#include <innodb/univ/univ.h>

/** Determine if the offsets are for a record in the new
 compact format.
 @return nonzero if compact format */
ulint rec_offs_comp(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
    MY_ATTRIBUTE((warn_unused_result));
