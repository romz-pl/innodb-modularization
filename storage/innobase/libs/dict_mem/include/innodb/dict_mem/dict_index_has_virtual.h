#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Check whether the index contains a virtual column
@param[in]	index	index
@return	nonzero for the index has virtual column, zero for other indexes */
UNIV_INLINE
ulint dict_index_has_virtual(const dict_index_t *index) {
  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  return (index->type & DICT_VIRTUAL);
}
