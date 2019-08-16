#include <innodb/record/rec_get_converted_size_comp.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/data_types/dfield_t.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_converted_size_comp_prefix_low.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree_nonleaf.h>

/** Determines the size of a data tuple in ROW_FORMAT=COMPACT.
 @return total size */
ulint rec_get_converted_size_comp(
    const dict_index_t *index, /*!< in: record descriptor;
                               dict_table_is_comp() is
                               assumed to hold, even if
                               it does not */
    ulint status,              /*!< in: status bits of the record */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of data fields */
    ulint *extra)              /*!< out: extra size */
{
  ulint size;
  ut_ad(n_fields > 0);

  switch (UNIV_EXPECT(status, REC_STATUS_ORDINARY)) {
    case REC_STATUS_ORDINARY:
      /* If this is a record for instant index, it could has
      less fields when it comes from update path */
      ut_ad(n_fields == dict_index_get_n_fields(index) ||
            index->has_instant_cols());
      size = 0;
      break;
    case REC_STATUS_NODE_PTR:
      n_fields--;
      ut_ad(n_fields == dict_index_get_n_unique_in_tree_nonleaf(index));
      ut_ad(dfield_get_len(&fields[n_fields]) == REC_NODE_PTR_SIZE);
      size = REC_NODE_PTR_SIZE; /* child page number */
      break;
    case REC_STATUS_INFIMUM:
    case REC_STATUS_SUPREMUM:
      /* infimum or supremum record, 8 data bytes */
      if (UNIV_LIKELY_NULL(extra)) {
        *extra = REC_N_NEW_EXTRA_BYTES;
      }
      return (REC_N_NEW_EXTRA_BYTES + 8);
    default:
      ut_error;
  }

  return (size + rec_get_converted_size_comp_prefix_low(
                     index, fields, n_fields, NULL, extra, &status, false));
}

