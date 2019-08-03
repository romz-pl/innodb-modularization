#pragma once

#include <innodb/univ/univ.h>

/** The following function returns the data size of a physical
 record, that is the sum of field lengths. SQL null fields
 are counted as length 0 fields. The value returned by the function
 is the distance from record origin to record end in bytes.
 @return size */
ulint rec_offs_data_size(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */

