#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;

/** Determine how many of the first n columns in a compact
 physical record are stored externally.
 @return number of externally stored columns */
ulint rec_get_n_extern_new(
    const rec_t *rec,          /*!< in: compact physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint n)                   /*!< in: number of columns to scan */
    MY_ATTRIBUTE((warn_unused_result));
