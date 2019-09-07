#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/purge_pq_t.h>

/** Create the memory copies for rollback segments and initialize the
rseg array in trx_sys at a database startup.
@param[in]	purge_queue	queue of rsegs to purge */
void trx_rsegs_init(purge_pq_t *purge_queue);
