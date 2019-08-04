#pragma once

#include <innodb/univ/univ.h>

struct hash_table_t;

#ifndef UNIV_HOTBACKUP

/** Releases all the locks of a hash table, in an ascending order. */
void hash_unlock_x_all(hash_table_t *table); /*!< in: hash table */

#else

#define hash_unlock_x_all(t) ((void)0)

#endif
