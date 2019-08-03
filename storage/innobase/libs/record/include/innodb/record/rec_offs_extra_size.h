#pragma once

#include <innodb/univ/univ.h>

/** Returns the total size of record minus data size of record. The value
 returned by the function is the distance from record start to record origin
 in bytes.
 @return size */
ulint rec_offs_extra_size(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */
