#pragma once

#include <innodb/univ/univ.h>

/** Returns the extra size of an old-style physical record if we know its
 data size and number of fields.
 @return extra size */
ulint rec_get_converted_extra_size(
    ulint data_size, /*!< in: data size */
    ulint n_fields,  /*!< in: number of fields */
    ulint n_ext);     /*!< in: number of externally stored columns */
