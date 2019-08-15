#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/dict_mem/dict_index_t.h>

/** Gets the column position in the clustered index. */
UNIV_INLINE
ulint dict_col_get_clust_pos(
    const dict_col_t *col,           /*!< in: table column */
    const dict_index_t *clust_index) /*!< in: clustered index */
{
  ulint i;

  ut_ad(col);
  ut_ad(clust_index);
  ut_ad(clust_index->is_clustered());

  for (i = 0; i < clust_index->n_def; i++) {
    const dict_field_t *field = &clust_index->fields[i];

    if (!field->prefix_len && field->col == col) {
      return (i);
    }
  }

  return (ULINT_UNDEFINED);
}
