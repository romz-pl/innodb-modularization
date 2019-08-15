#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Gets the approximately estimated number of rows in the table.
 @return estimated number of rows */
UNIV_INLINE
ib_uint64_t dict_table_get_n_rows(const dict_table_t *table) /*!< in: table */
{
  ut_ad(table->stat_initialized);

  return (table->stat_n_rows);
}
