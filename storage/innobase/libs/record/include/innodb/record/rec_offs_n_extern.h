#pragma once

#include <innodb/univ/univ.h>

/** Returns the number of extern bits set in a record.
 @return number of externally stored fields */
ulint rec_offs_n_extern(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */
