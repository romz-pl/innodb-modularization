#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/sync_mutex/ib_mutex_t.h>

struct hash_table_t;

/** Gets the nth mutex in a hash table.
@param[in]	table	hash table
@param[in]	i	index of the mutex
@return mutex */
ib_mutex_t *hash_get_nth_mutex(hash_table_t *table, ulint i);

#endif
