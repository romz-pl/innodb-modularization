#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/rec_get_n_fields_old.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree_nonleaf.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>

/** The following function is used to get the number of fields
in an old-style record. Have to consider the case that after
instant ADD COLUMN, this record may have less fields than
current index.
@param[in]	rec	physical record
@param[in]	index	index where the record resides
@return number of data fields */
UNIV_INLINE MY_ATTRIBUTE((warn_unused_result)) uint16_t
    rec_get_n_fields_old(const rec_t *rec, const dict_index_t *index) {
  uint16_t n = rec_get_n_fields_old_raw(rec);

  if (index->has_instant_cols()) {
    uint16_t n_uniq = dict_index_get_n_unique_in_tree_nonleaf(index);

    ut_ad(index->is_clustered());
    ut_ad(n <= dict_index_get_n_fields(index));
    ut_ad(n_uniq > 0);
    /* Only when it's infimum or supremum, n is 1.
    If n is exact n_uniq, this should be a record copied with prefix during
    search.
    And if it's node pointer, n is n_uniq + 1, which should be always less
    than the number of fields in any leaf page, even if the record in
    leaf page is before instant ADD COLUMN. This is because any record in
    leaf page must have at least n_uniq + 2 (system columns) fields */
    ut_ad(n == 1 || n >= n_uniq);
    ut_ad(static_cast<uint16_t>(dict_index_get_n_fields(index)) > n_uniq + 1);
    if (n > n_uniq + 1) {
#ifdef UNIV_DEBUG
      ulint rec_diff = dict_index_get_n_fields(index) - n;
      ulint col_diff = index->table->n_cols - index->table->n_instant_cols;
      ut_ad(rec_diff <= col_diff);
      if (n != dict_index_get_n_fields(index)) {
        ut_ad(index->has_instant_cols());
      }
#endif /* UNIV_DEBUG */
      n = static_cast<uint16_t>(dict_index_get_n_fields(index));
    }
  }

  return (n);
}
