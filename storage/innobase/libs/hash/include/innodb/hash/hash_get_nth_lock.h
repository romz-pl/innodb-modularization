#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

#ifndef UNIV_HOTBACKUP

struct hash_table_t;

/** Gets the nth rw_lock in a hash table.
@param[in]	table	hash table
@param[in]	i	index of the mutex
@return rw_lock */
rw_lock_t *hash_get_nth_lock(hash_table_t *table, ulint i);


#endif
