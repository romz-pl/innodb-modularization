#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Check whether the index is a Spatial Index.
 @return	nonzero for Spatial Index, zero for other indexes */
UNIV_INLINE
ulint dict_index_is_spatial(const dict_index_t *index) /*!< in: index */
{
  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  return (index->type & DICT_SPATIAL);
}
