#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree.h>
#include <innodb/dict_mem/dict_index_is_spatial.h>
#include <innodb/dict_mem/flags.h>

/**
Gets the number of fields on nonleaf page level in the internal representation
of an index which uniquely determine the position of an index entry in the
index, if we also take multiversioning into account. Note, it doesn't
include page no field.
@param[in]	index	index
@return number of fields */
UNIV_INLINE
uint16_t dict_index_get_n_unique_in_tree_nonleaf(const dict_index_t *index) {
  ut_ad(index != NULL);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);
  ut_ad(index->cached);

  if (dict_index_is_spatial(index)) {
    /* For spatial index, on non-leaf page, we have only
    2 fields(mbr+page_no). So, except page no field,
    there's one field there. */
    return (DICT_INDEX_SPATIAL_NODEPTR_SIZE);
  } else {
    return static_cast<uint16_t>(dict_index_get_n_unique_in_tree(index));
  }
}
