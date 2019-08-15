#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/table_lock_list_t.h>

/** Initialise the table lock list. */
void lock_table_lock_list_init(
    table_lock_list_t *locks); /*!< List to initialise */
