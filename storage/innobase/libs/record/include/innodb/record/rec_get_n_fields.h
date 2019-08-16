#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree.h>

#include <innodb/record/rec_get_n_fields_old.h>
#include <innodb/record/rec_get_status.h>
#include <innodb/record/flag.h>


/** The following function is used to get the number of fields
 in a record. If it's REDUNDANT record, the returned number
 would be a logic one which considers the fact that after
 instant ADD COLUMN, some records may have less fields than
 index.
 @return number of data fields */
UNIV_INLINE
ulint rec_get_n_fields(const rec_t *rec,          /*!< in: physical record */
                       const dict_index_t *index) /*!< in: record descriptor */
{
  ut_ad(rec);
  ut_ad(index);

  if (!dict_table_is_comp(index->table)) {
    return (rec_get_n_fields_old(rec, index));
  }

  switch (rec_get_status(rec)) {
    case REC_STATUS_ORDINARY:
      return (dict_index_get_n_fields(index));
    case REC_STATUS_NODE_PTR:
      return (dict_index_get_n_unique_in_tree(index) + 1);
    case REC_STATUS_INFIMUM:
    case REC_STATUS_SUPREMUM:
      return (1);
    default:
      ut_error;
  }
}
