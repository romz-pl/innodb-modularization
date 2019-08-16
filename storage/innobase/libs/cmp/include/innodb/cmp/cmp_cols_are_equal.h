#pragma once

#include <innodb/univ/univ.h>

struct dict_col_t;

/** Returns TRUE if two columns are equal for comparison purposes.
 @return true if the columns are considered equal in comparisons */
ibool cmp_cols_are_equal(const dict_col_t *col1, /*!< in: column 1 */
                         const dict_col_t *col2, /*!< in: column 2 */
                         ibool check_charsets);
