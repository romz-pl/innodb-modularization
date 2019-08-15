#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Mark if table has big rows.
@param[in,out]	table	table handler */
void dict_table_set_big_rows(dict_table_t *table) MY_ATTRIBUTE((nonnull));
