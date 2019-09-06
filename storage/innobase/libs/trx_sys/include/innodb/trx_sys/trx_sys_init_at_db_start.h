#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/purge_pq_t.h>

/** Creates and initializes the central memory structures for the transaction
 system. This is called when the database is started.
 @return min binary heap of rsegs to purge */
purge_pq_t *trx_sys_init_at_db_start(void);
