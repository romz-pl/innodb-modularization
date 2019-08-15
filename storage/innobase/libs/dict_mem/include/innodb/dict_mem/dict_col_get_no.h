#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_col_t.h>

/** Gets the column number.
 @return col->ind, table column position (starting from 0) */
UNIV_INLINE
ulint dict_col_get_no(const dict_col_t *col) /*!< in: column */
{
  ut_ad(col);

  return (col->ind);
}
