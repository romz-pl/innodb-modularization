#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Move a table to the LRU end from the non LRU list.
@param[in]	table	InnoDB table object */
void dict_table_move_from_non_lru_to_lru(dict_table_t *table);
