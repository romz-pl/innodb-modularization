#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Free the vcol_set from all foreign key constraint on the table.
@param[in,out]	table	innodb table object. */
void dict_mem_table_free_foreign_vcol_set(dict_table_t *table);
