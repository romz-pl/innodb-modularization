#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/sync_mutex/ib_mutex_t.h>

struct hash_table_t;

/** Gets the mutex for a fold value in a hash table.
@param[in]	table	hash table
@param[in]	fold	fold
@return mutex */
ib_mutex_t *hash_get_mutex(hash_table_t *table, ulint fold);


#endif
