#pragma once

#include <innodb/univ/univ.h>

struct hash_table_t;

/** Frees a hash table. */
void hash_table_free(hash_table_t *table); /*!< in, own: hash table */

