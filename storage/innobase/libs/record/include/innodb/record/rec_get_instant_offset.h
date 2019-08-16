#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/record/flag.h>


/** Determine the offset of a specified field in the record, when this
field is a field added after an instant ADD COLUMN
@param[in]	index	Clustered index where the record resides
@param[in]	n	Nth field to get offset
@param[in]	offs	Last offset before current field
@return The offset of the specified field */
UNIV_INLINE
uint64_t rec_get_instant_offset(const dict_index_t *index, ulint n,
                                uint64_t offs) {
  ut_ad(index->has_instant_cols());

  ulint length;
  index->get_nth_default(n, &length);

  if (length == UNIV_SQL_NULL) {
    return (offs | REC_OFFS_SQL_NULL);
  } else {
    return (offs | REC_OFFS_DEFAULT);
  }
}
