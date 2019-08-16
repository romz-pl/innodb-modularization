#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/record/rec_get_n_fields.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_types/flags.h>

/** Confirms the n_fields of the entry is sane with comparing the other
record in the same page specified
@param[in]	index	index
@param[in]	rec	record of the same page
@param[in]	entry	index entry
@return	true if n_fields is sane */
UNIV_INLINE
bool rec_n_fields_is_sane(dict_index_t *index, const rec_t *rec,
                          const dtuple_t *entry) {
  return (rec_get_n_fields(rec, index) == dtuple_get_n_fields(entry)
          /* a record for older SYS_INDEXES table
          (missing merge_threshold column) is acceptable. */
          || (index->table->id == DICT_INDEXES_ID &&
              rec_get_n_fields(rec, index) == dtuple_get_n_fields(entry) - 1));
}
