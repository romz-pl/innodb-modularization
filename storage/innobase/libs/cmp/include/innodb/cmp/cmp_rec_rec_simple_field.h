#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;

MY_ATTRIBUTE((warn_unused_result)) int cmp_rec_rec_simple_field(
    const rec_t *rec1,         /*!< in: physical record */
    const rec_t *rec2,         /*!< in: physical record */
    const ulint *offsets1,     /*!< in: rec_get_offsets(rec1, ...) */
    const ulint *offsets2,     /*!< in: rec_get_offsets(rec2, ...) */
    const dict_index_t *index, /*!< in: data dictionary index */
    ulint n);                   /*!< in: field to compare */
