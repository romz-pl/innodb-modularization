#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Check whether the index consists of descending columns only.
@param[in]	index  index tree
@retval true if index has any descending column
@retval false if index has only ascending columns */
UNIV_INLINE
bool dict_index_has_desc(const dict_index_t *index) {
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  for (ulint i = 0; i < index->n_def; i++) {
    const dict_field_t *field = &index->fields[i];

    if (!field->is_ascending) {
      return (true);
    }
  }

  return (false);
}
