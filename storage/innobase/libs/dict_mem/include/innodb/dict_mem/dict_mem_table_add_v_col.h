#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dict_table_t;
struct dict_v_col_t;

/** Adds a virtual column definition to a table.
@param[in,out]	table		table
@param[in]	heap		temporary memory heap, or NULL. It is
                                used to store name when we have not finished
                                adding all columns. When all columns are
                                added, the whole name will copy to memory from
                                table->heap
@param[in]	name		column name
@param[in]	mtype		main datatype
@param[in]	prtype		precise type
@param[in]	len		length
@param[in]	pos		position in a table
@param[in]	num_base	number of base columns
@return the virtual column definition */
dict_v_col_t *dict_mem_table_add_v_col(dict_table_t *table, mem_heap_t *heap,
                                       const char *name, ulint mtype,
                                       ulint prtype, ulint len, ulint pos,
                                       ulint num_base);
