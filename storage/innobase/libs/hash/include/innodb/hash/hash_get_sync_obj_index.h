#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct hash_table_t;

ulint hash_get_sync_obj_index(hash_table_t *table, /*!< in: hash table */
                              ulint fold);          /*!< in: fold */

#endif
