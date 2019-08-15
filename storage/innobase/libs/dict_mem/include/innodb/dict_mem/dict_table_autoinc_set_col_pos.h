#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_mysql_pos_to_innodb.h>
#include <innodb/dict_mem/dict_table_get_nth_col_pos.h>
#include <innodb/dict_mem/dict_table_mysql_pos_to_innodb.h>

/** Set the column position of autoinc column in clustered index for a table.
@param[in]	table	table
@param[in]	pos	column position in table definition */
UNIV_INLINE
void dict_table_autoinc_set_col_pos(dict_table_t *table, ulint pos) {
  ulint innodb_pos = dict_table_mysql_pos_to_innodb(table, pos);

  ulint idx = dict_table_get_nth_col_pos(table, innodb_pos);

  table->autoinc_field_no = idx;
}
