#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;
struct ib_vector_t;

/** Get all the FTS indexes on a table.
@param[in]	table	table
@param[out]	indexes	all FTS indexes on this table
@return number of FTS indexes */
ulint dict_table_get_all_fts_indexes(dict_table_t *table, ib_vector_t *indexes);
