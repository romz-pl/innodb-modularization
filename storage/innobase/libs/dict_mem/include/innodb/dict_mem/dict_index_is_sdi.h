#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/flags.h>

/** Check if the index is SDI index
@param[in]	index	in-memory index structure
@return true if index is SDI index else false */
UNIV_INLINE
bool dict_index_is_sdi(const dict_index_t *index) {
  return (index->type & DICT_SDI);
}
