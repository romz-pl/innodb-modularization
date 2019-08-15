#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Returns a virtual column's name.
@param[in]	table		table object
@param[in]	col_nr		virtual column number(nth virtual column)
@return column name. */
const char *dict_table_get_v_col_name(const dict_table_t *table, ulint col_nr);
