#pragma once

#include <innodb/univ/univ.h>

/** Generate a table_id from space id for SDI Index.
@param[in]	space_id	InnoDB tablespace id
@return table_id */
UNIV_INLINE
uint64_t dict_sdi_get_table_id(space_id_t space_id) {
  return ((~0ULL << 32) | space_id);
}
