#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/sync_rw/rw_lock_t.h>

struct hash_table_t;

/** If not appropriate rw_lock for a fold value in a hash table,
relock X-lock the another rw_lock until appropriate for a fold value.
@param[in]	hash_lock	latched rw_lock to be confirmed
@param[in]	table		hash table
@param[in]	fold		fold value
@return	latched rw_lock */
rw_lock_t *hash_lock_x_confirm(rw_lock_t *hash_lock, hash_table_t *table,
                               ulint fold);

#endif
