#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;
struct dtuple_t;

/** Builds a physical record out of a data tuple and
 stores it into the given buffer.
 @return pointer to the origin of physical record */
rec_t *rec_convert_dtuple_to_rec(
    byte *buf,                 /*!< in: start address of the
                               physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    const dtuple_t *dtuple,    /*!< in: data tuple */
    ulint n_ext)               /*!< in: number of
                               externally stored columns */
    MY_ATTRIBUTE((warn_unused_result));
