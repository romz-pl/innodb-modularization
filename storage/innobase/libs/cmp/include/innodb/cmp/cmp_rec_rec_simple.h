#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

#include "sql/table.h"

struct dict_index_t;

/** Compare two physical records that contain the same number of columns,
none of which are stored externally.
@retval positive if rec1 (including non-ordering columns) is greater than rec2
@retval negative if rec1 (including non-ordering columns) is less than rec2
@retval 0 if rec1 is a duplicate of rec2 */
int cmp_rec_rec_simple(
    const rec_t *rec1,         /*!< in: physical record */
    const rec_t *rec2,         /*!< in: physical record */
    const ulint *offsets1,     /*!< in: rec_get_offsets(rec1, ...) */
    const ulint *offsets2,     /*!< in: rec_get_offsets(rec2, ...) */
    const dict_index_t *index, /*!< in: data dictionary index */
    struct TABLE *table)       /*!< in: MySQL table, for reporting
                               duplicate key value if applicable,
                               or NULL */
    MY_ATTRIBUTE((warn_unused_result));
