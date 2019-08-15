#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Fill virtual columns set in each fk constraint present in the table.
@param[in,out]	table	innodb table object. */
void dict_mem_table_fill_foreign_vcol_set(dict_table_t *table);
