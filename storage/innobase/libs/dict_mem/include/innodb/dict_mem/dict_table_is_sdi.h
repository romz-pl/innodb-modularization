#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_sdi_get_table_id.h>
#include <innodb/dict_mem/flags.h>

/** Check if an table id belongs SDI table
@param[in]	table_id	dict_table_t id
@return true if table_id is SDI table_id else false */
UNIV_INLINE
bool dict_table_is_sdi(table_id_t table_id) {
  /* The lowest possible SDI table_id is for space 0.
  So any table id greater than system tablespace
  table id is SDI table id. */
  return (table_id >= dict_sdi_get_table_id(SYSTEM_TABLE_SPACE));
}
