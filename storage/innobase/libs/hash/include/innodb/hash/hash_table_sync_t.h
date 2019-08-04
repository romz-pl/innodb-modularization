#pragma once

#include <innodb/univ/univ.h>

/* Differnt types of hash_table based on the synchronization
method used for it. */
enum hash_table_sync_t {
  HASH_TABLE_SYNC_NONE = 0, /*!< Don't use any internal
                            synchronization objects for
                            this hash_table. */
  HASH_TABLE_SYNC_MUTEX,    /*!< Use mutexes to control
                            access to this hash_table. */
  HASH_TABLE_SYNC_RW_LOCK   /*!< Use rw_locks to control
                            access to this hash_table. */
};
