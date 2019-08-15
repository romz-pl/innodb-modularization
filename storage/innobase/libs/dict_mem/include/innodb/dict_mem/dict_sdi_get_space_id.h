#pragma once

#include <innodb/univ/univ.h>

/** Extract space_id from table_id for SDI Index.
@param[in]	table_id	InnoDB table id
@return space_id */
UNIV_INLINE
space_id_t dict_sdi_get_space_id(table_id_t table_id) {
  return (static_cast<space_id_t>(table_id));
}
