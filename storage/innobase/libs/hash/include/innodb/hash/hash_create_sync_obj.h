#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_table_sync_t.h>
#include <innodb/sync_latch/latch_id_t.h>

struct hash_table_t;

/** Creates a sync object array array to protect a hash table. "::sync_obj"
can be mutexes or rw_locks depening on the type of hash table.
@param[in]	table		hash table
@param[in]	type		HASH_TABLE_SYNC_MUTEX or HASH_TABLE_SYNC_RW_LOCK
@param[in]	id		mutex/rw_lock ID
@param[in]	n_sync_obj	number of sync objects, must be a power of 2*/
void hash_create_sync_obj(hash_table_t *table, hash_table_sync_t type,
                          latch_id_t id, ulint n_sync_obj);


#endif /* !UNIV_HOTBACKUP */
