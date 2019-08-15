#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;

/** Looks for a matching field in an index. The column has to be the same. The
 column in index must be complete, or must contain a prefix longer than the
 column in index2. That is, we must be able to construct the prefix in index2
 from the prefix in index.
 @return position in internal representation of the index;
 ULINT_UNDEFINED if not contained */
ulint dict_index_get_nth_field_pos(
    const dict_index_t *index,  /*!< in: index from which to search */
    const dict_index_t *index2, /*!< in: index */
    ulint n)                    /*!< in: field number in index2 */
    MY_ATTRIBUTE((warn_unused_result));
