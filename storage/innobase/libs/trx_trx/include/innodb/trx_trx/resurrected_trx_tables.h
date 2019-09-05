#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_table_map.h>

/** Map of resurrected transactions to affected table_id */
extern trx_table_map resurrected_trx_tables;
