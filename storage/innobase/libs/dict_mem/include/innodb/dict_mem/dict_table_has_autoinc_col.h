#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Check if a table has an autoinc counter column.
@param[in]	table	table
@return true if there is an autoinc column in the table, otherwise false. */
UNIV_INLINE
bool dict_table_has_autoinc_col(const dict_table_t *table) {
  return (table->autoinc_field_no != ULINT_UNDEFINED);
}
