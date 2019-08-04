#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct hash_table_t;

#include <innodb/sync_rw/rw_lock_t.h>

rw_lock_t *hash_get_lock(hash_table_t *table, /*!< in: hash table */
                         ulint fold);          /*!< in: fold */

#endif
