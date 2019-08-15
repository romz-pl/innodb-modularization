#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_get_n_unique.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>

/** Gets the number of fields in the internal representation of an index
 which uniquely determine the position of an index entry in the index, if
 we also take multiversioning into account.
 @return number of fields */
UNIV_INLINE
ulint dict_index_get_n_unique_in_tree(
    const dict_index_t *index) /*!< in: an internal representation
                               of index (in the dictionary cache) */
{
  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
  ut_ad(index->cached);

  if (index->is_clustered()) {
    return (dict_index_get_n_unique(index));
  }

  return (static_cast<uint16_t>(dict_index_get_n_fields(index)));
}
