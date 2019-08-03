#pragma once

#include <innodb/univ/univ.h>


/** The following function returns the number of fields in a record.
 @return number of fields */
MY_ATTRIBUTE((warn_unused_result)) ulint rec_offs_n_fields(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */
