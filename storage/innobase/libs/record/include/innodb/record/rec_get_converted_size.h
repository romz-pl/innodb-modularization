#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;
struct dtuple_t;

/** The following function returns the size of a data tuple when converted to
 a physical record.
 @return size */
ulint rec_get_converted_size(
    const dict_index_t *index, /*!< in: record descriptor */
    const dtuple_t *dtuple,    /*!< in: data tuple */
    ulint n_ext)               /*!< in: number of externally stored columns */
    MY_ATTRIBUTE((warn_unused_result));
