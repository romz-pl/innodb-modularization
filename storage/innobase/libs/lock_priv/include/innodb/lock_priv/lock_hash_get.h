#pragma once

#include <innodb/univ/univ.h>

struct hash_table_t;

/** Get the lock hash table */
hash_table_t *lock_hash_get(ulint mode); /*!< in: lock mode */
