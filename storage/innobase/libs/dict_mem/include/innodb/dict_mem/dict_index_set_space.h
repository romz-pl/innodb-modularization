#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Sets the space id of the root of the index tree. */
UNIV_INLINE
void dict_index_set_space(dict_index_t *index, /*!< in/out: index */
                          space_id_t space)    /*!< in: space id */
{
  ut_ad(index);
  ut_ad(index->magic_n == DICT_INDEX_MAGIC_N);

  index->space = space;
}
