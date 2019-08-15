#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Checks if a column is in the ordering columns of the clustered index of a
 table. Column prefixes are treated like whole columns.
 @return true if the column, or its prefix, is in the clustered key */
ibool dict_table_col_in_clustered_key(
    const dict_table_t *table, /*!< in: table */
    ulint n)                   /*!< in: column number */
    MY_ATTRIBUTE((warn_unused_result));
