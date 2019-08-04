#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_t.h>

struct hash_table_t;

#ifndef UNIV_HOTBACKUP

/** Releases all but passed in lock of a hash table, */
void hash_unlock_x_all_but(hash_table_t *table,   /*!< in: hash table */
                           rw_lock_t *keep_lock); /*!< in: lock to keep */


#else

#define hash_unlock_x_all_but(t, l) ((void)0)

#endif
