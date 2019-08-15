#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Gets the number of fields in the internal representation of an index
 that uniquely determine the position of an index entry in the index, if
 we do not take multiversioning into account: in the B-tree use the value
 returned by dict_index_get_n_unique_in_tree.
 @return number of fields */
UNIV_INLINE
ulint dict_index_get_n_unique(
    const dict_index_t *index) /*!< in: an internal representation
                               of index (in the dictionary cache) */
{
  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
  ut_ad(index->cached);

  return static_cast<uint16_t>(index->n_uniq);
}
