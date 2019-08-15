#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Determine if a table uses atomic BLOBs (no locally stored prefix).
@param[in]	table	InnoDB table
@return whether BLOBs are atomic */
UNIV_INLINE
bool dict_table_has_atomic_blobs(const dict_table_t *table) {
  return (DICT_TF_HAS_ATOMIC_BLOBS(table->flags));
}
