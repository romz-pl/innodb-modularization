#pragma once

#include <innodb/univ/univ.h>

/** Determine if the offsets are for a record containing
 externally stored columns.
 @return nonzero if externally stored */
ulint rec_offs_any_extern(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
    MY_ATTRIBUTE((warn_unused_result));
