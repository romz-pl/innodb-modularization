#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/table_id_set.h>

#include <map>

/** Map of transactions to affected table_id */
typedef std::map<trx_t *, table_id_set, std::less<trx_t *>,
                 ut_allocator<std::pair<trx_t *const, table_id_set>>>
    trx_table_map;
