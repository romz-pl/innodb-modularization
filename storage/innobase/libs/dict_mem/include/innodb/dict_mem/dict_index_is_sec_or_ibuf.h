#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Check whether the index is a secondary index or the insert buffer tree.
 @return nonzero for insert buffer, zero for other indexes */
UNIV_INLINE
ulint dict_index_is_sec_or_ibuf(const dict_index_t *index) /*!< in: index */
{
  ulint type;

  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  type = index->type;

  return (!(type & DICT_CLUSTERED) || (type & DICT_IBUF));
}
