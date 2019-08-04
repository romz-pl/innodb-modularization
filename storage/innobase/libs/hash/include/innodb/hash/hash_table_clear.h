#pragma once

#include <innodb/univ/univ.h>

struct hash_table_t;

void hash_table_clear(hash_table_t *table); /*!< in/out: hash table */
