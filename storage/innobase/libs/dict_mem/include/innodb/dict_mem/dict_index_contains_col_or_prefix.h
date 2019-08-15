#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;

/** Returns TRUE if the index contains a column or a prefix of that column.
 @return true if contains the column or its prefix */
ibool dict_index_contains_col_or_prefix(
    const dict_index_t *index, /*!< in: index */
    ulint n,                   /*!< in: column number */
    bool is_virtual)
    /*!< in: whether it is a virtual col */
    MY_ATTRIBUTE((warn_unused_result));
