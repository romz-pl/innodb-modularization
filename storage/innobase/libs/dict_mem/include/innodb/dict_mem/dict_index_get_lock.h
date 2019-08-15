#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Gets the read-write lock of the index tree.
 @return read-write lock */
UNIV_INLINE
rw_lock_t *dict_index_get_lock(dict_index_t *index) /*!< in: index */
{
  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  return (&(index->lock));
}
