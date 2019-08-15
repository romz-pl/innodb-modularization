#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Looks for non-virtual column n position in the clustered index.
 @return position in internal representation of the clustered index */
ulint dict_table_get_nth_col_pos(const dict_table_t *table, /*!< in: table */
                                 ulint n) /*!< in: column number */
    MY_ATTRIBUTE((warn_unused_result));
