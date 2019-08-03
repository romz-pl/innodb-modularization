#pragma once

#include <innodb/univ/univ.h>

/** Returns nonzero if the SQL NULL bit is set in nth field of rec.
 @return nonzero if SQL NULL */
ulint rec_offs_nth_sql_null(
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n);              /*!< in: nth field */
