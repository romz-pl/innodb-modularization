#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Move to the most recently used segment of the LRU list. */
void dict_move_to_mru(dict_table_t *table); /*!< in: table to move to MRU */
