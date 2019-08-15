#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;
struct dict_index_t;
struct dict_add_v_col_t;

/** Tries to find column names for the index and sets the col field of the
index.
@param[in]	table	table
@param[in]	index	index
@param[in]	add_v	new virtual columns added along with an add index call
@return true if the column names were found */
ibool dict_index_find_cols(const dict_table_t *table,
                                  dict_index_t *index,
                                  const dict_add_v_col_t *add_v);
