#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_contains_col_or_prefix.h>
#include <innodb/vector/vector.h>

/** Check whether a column exists in an FTS index.
 @return ULINT_UNDEFINED if no match else the offset within the vector */
UNIV_INLINE
ulint dict_table_is_fts_column(
    ib_vector_t *indexes, /*!< in: vector containing only FTS indexes */
    ulint col_no,         /*!< in: col number to search for */
    bool is_virtual)      /*!< in: whether it is a virtual column */

{
  ulint i;

  for (i = 0; i < ib_vector_size(indexes); ++i) {
    dict_index_t *index;

    index = (dict_index_t *)ib_vector_getp(indexes, i);

    if (dict_index_contains_col_or_prefix(index, col_no, is_virtual)) {
      return (i);
    }
  }

  return (ULINT_UNDEFINED);
}
