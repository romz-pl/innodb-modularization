#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;
struct dfield_t;
struct dtuple_t;

/** Determines the size of a data tuple prefix in a temporary file.
 @return total size */
ulint rec_get_converted_size_temp(
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of data fields */
    const dtuple_t *v_entry,   /*!< in: dtuple contains virtual column
                               data */
    ulint *extra)              /*!< out: extra size */
    MY_ATTRIBUTE((warn_unused_result));
