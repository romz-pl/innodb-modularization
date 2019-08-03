#pragma once

#include <innodb/univ/univ.h>

/** Returns nonzero if the extern bit is set in nth field of rec.
 @return nonzero if externally stored */
ulint rec_offs_nth_extern(
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n);              /*!< in: nth field */
