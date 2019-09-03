#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <set>

struct dict_table_t;

/** Type used to store the list of tables that are modified by a given
transaction. We store pointers to the table objects in memory because
we know that a table object will not be destroyed while a transaction
that modified it is running. */
typedef std::set<dict_table_t *, std::less<dict_table_t *>,
                 ut_allocator<dict_table_t *>>
    trx_mod_tables_t;
