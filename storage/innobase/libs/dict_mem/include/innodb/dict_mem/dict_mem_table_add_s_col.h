#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Adds a stored column definition to a table.
@param[in,out]	table		table
@param[in]	num_base	number of base columns. */
void dict_mem_table_add_s_col(dict_table_t *table, ulint num_base);
