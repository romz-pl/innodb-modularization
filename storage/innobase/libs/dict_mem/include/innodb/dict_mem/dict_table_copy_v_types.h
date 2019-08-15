#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;
struct dtuple_t;

/** Copies types of virtual columns contained in table to tuple and sets all
fields of the tuple to the SQL NULL value.  This function should
be called right after dtuple_create().
@param[in,out]	tuple	data tuple
@param[in]	table	table */
void dict_table_copy_v_types(dtuple_t *tuple, const dict_table_t *table);
