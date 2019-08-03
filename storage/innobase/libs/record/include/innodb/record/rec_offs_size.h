#pragma once

#include <innodb/univ/univ.h>

/** Returns the total size of a physical record.
 @return size */
ulint rec_offs_size(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */
