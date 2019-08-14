#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

#ifdef UNIV_DEBUG

/** Validates offsets returned by rec_get_offsets().
 @return true if valid */
UNIV_INLINE MY_ATTRIBUTE((warn_unused_result)) ibool rec_offs_validate(
    const rec_t *rec,          /*!< in: record or NULL */
    const dict_index_t *index, /*!< in: record descriptor or NULL */
    const ulint *offsets)      /*!< in: array returned by
                               rec_get_offsets() */
{
  ulint i = rec_offs_n_fields(offsets);
  ulint last = ULINT_MAX;
  ulint comp = *rec_offs_base(offsets) & REC_OFFS_COMPACT;

  if (rec) {
    ut_ad((ulint)rec == offsets[2]);
    if (!comp && index != nullptr) {
      ut_a(rec_get_n_fields_old(rec, index) >= i);
    }
  }
  if (index) {
    ulint max_n_fields;
    ut_ad((ulint)index == offsets[3]);
    ulint n_fields = dict_index_get_n_fields(index);
    ulint n_unique_in_tree = dict_index_get_n_unique_in_tree(index) + 1;
    max_n_fields = std::max(n_fields, n_unique_in_tree);
    if (!comp && rec != nullptr && rec_get_n_fields_old_raw(rec) < i) {
      ut_a(index->has_instant_cols());
    }

    if (comp && rec) {
      switch (rec_get_status(rec)) {
        case REC_STATUS_ORDINARY:
          break;
        case REC_STATUS_NODE_PTR:
          max_n_fields = dict_index_get_n_unique_in_tree(index) + 1;
          break;
        case REC_STATUS_INFIMUM:
        case REC_STATUS_SUPREMUM:
          max_n_fields = 1;
          break;
        default:
          ut_error;
      }
    }
    /* index->n_def == 0 for dummy indexes if !comp */
    ut_a(!comp || index->n_def);
    ut_a(!index->n_def || i <= max_n_fields);
  }
  while (i--) {
    ulint curr = rec_offs_base(offsets)[1 + i] & REC_OFFS_MASK;
    ut_a(curr <= last);
    last = curr;
  }
  return (TRUE);
}


#endif
