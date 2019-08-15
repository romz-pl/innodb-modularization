#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>

/** Check if index is auto-generated clustered index.
@param[in]	index	index

@return true if index is auto-generated clustered index. */
UNIV_INLINE
bool dict_index_is_auto_gen_clust(const dict_index_t *index) {
  return (index->type == DICT_CLUSTERED);
}
