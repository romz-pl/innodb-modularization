#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Move a table to the non LRU end of the LRU list. */
void dict_table_move_from_lru_to_non_lru(
    dict_table_t *table); /*!< in: table to move from LRU to non-LRU */
