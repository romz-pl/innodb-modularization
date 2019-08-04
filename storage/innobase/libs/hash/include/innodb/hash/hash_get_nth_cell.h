#pragma once

#include <innodb/univ/univ.h>

struct hash_table_t;
struct hash_cell_t;

hash_cell_t *hash_get_nth_cell(hash_table_t *table, /*!< in: hash table */
                               ulint n);             /*!< in: cell index */
