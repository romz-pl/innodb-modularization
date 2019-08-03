#pragma once

#include <innodb/univ/univ.h>

/** Gets the physical size of a field.
 @return length of field */
ulint rec_offs_nth_size(
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n);              /*!< in: nth field */
