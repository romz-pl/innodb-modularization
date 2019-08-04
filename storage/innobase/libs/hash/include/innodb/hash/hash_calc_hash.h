#pragma once

#include <innodb/univ/univ.h>

struct hash_table_t;

ulint hash_calc_hash(ulint fold,          /*!< in: folded value */
                     hash_table_t *table); /*!< in: hash table */
