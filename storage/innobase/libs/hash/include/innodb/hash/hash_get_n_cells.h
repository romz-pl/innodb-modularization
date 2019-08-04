#pragma once

#include <innodb/univ/univ.h>

struct hash_table_t;

ulint hash_get_n_cells(hash_table_t *table); /*!< in: table */
