#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

#ifdef UNIV_DEBUG
/** Check if a table is a temporary table with compressed row format,
we should always expect false.
@param[in]	table	table
@return true if it's a compressed temporary table, false otherwise */
inline bool dict_table_is_compressed_temporary(const dict_table_t *table) {
  if (table->is_temporary()) {
    ut_ad(fsp_is_system_temporary(table->space));

    return (dict_table_page_size(table).is_compressed());
  }

  return (false);
}
#endif /* UNIV_DEBUG */
