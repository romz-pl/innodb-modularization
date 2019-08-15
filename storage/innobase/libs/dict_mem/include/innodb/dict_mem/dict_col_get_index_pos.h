#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/dict_mem/dict_index_t.h>

/** Gets the column position in the given index.
@param[in]	col	table column
@param[in]	index	index to be searched for column
@return position of column in the given index. */
UNIV_INLINE
ulint dict_col_get_index_pos(const dict_col_t *col, const dict_index_t *index) {
  ulint i;

  for (i = 0; i < index->n_def; i++) {
    const dict_field_t *field = &index->fields[i];

    if (!field->prefix_len && field->col == col) {
      return (i);
    }
  }

  return (ULINT_UNDEFINED);
}
